import { google } from "googleapis";
import dotenv from "dotenv";

dotenv.config({ path: `.env.${process.env.NODE_ENV || "development"}` });

const oauth2Client = new google.auth.OAuth2(
  process.env.GOOGLE_CLIENT_ID,
  process.env.GOOGLE_CLIENT_SECRET,
  process.env.GOOGLE_REDIRECT_URI
);
// Step 1: Redirect user to this route to start Gmail auth
export const loginToGmail = (req, res) => {
  const scopes = ["https://www.googleapis.com/auth/gmail.readonly"];
  const url = oauth2Client.generateAuthUrl({
    access_type: "offline",
    scope: scopes,
  });
  res.redirect(url);
};

import fs from "fs";

export const gmailCallback = async (req, res) => {
  const code = req.query.code;
  try {
    const { tokens } = await oauth2Client.getToken(code);
    oauth2Client.setCredentials(tokens);

    // Save tokens to a file for future use
    fs.writeFileSync("gmail_tokens.json", JSON.stringify(tokens));

    res.redirect("/api/gmail/emails");
  } catch (error) {
    console.error("Error during Gmail callback:", error);
    res.status(500).json({ message: "Gmail login failed" });
  }
};

// Step 3: Use the access token to read emails
export const getEmails = async (req, res) => {
  try {
    const tokenData = JSON.parse(fs.readFileSync("gmail_tokens.json"));
    oauth2Client.setCredentials(tokenData);

    const gmail = google.gmail({ version: "v1", auth: oauth2Client });

    const response = await gmail.users.messages.list({
      userId: "me",
      maxResults: 5,
    });

    const messages = await Promise.all(
      (response.data.messages || []).map(async (msg) => {
        const detail = await gmail.users.messages.get({
          userId: "me",
          id: msg.id,
        });
        return {
          id: msg.id,
          snippet: detail.data.snippet,
        };
      })
    );

    res.status(200).json({ messages });
  } catch (error) {
    console.error("Error fetching emails:", error.message);
    res.status(500).json({ message: "Failed to fetch emails" });
  }
};
