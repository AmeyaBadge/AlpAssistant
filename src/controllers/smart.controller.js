import axios from "axios";
import nlp from "compromise";

export const smartHandler = async (req, res) => {
  const { query } = req.query;
  console.log("Query: ", query);
  if (!query) {
    return res.status(400).json({ message: "Query is required" });
  }

  let doc = nlp(query.toLowerCase());
  try {
    // Check for weather
    if (
      query.includes("weather") ||
      doc.has("rain") ||
      doc.has("temperature") ||
      doc.has("hot")
    ) {
      console.log("Weather detected");
      const city = doc.match("#Place").text() || "Pune";
      console.log("City detected:", city);
      const response = await axios.get(
        `${process.env.BASE_URL}/weather?city=${city}`
      );
      return res.status(200).json({ action: "weather", data: response.data });
    }

    // Check for news
    else if (
      query.includes("news") ||
      doc.has("headline") ||
      doc.has("updates")
    ) {
      const category = doc.match("#Noun").last().text() || "general";
      console.log("Category detected:", category);
      const response = await axios.get(
        `${process.env.BASE_URL}/news?category=${category}`
      );
      return res.status(200).json({ action: "news", data: response.data });
    }

    // Check for email
    if (query.includes("email") || doc.has("mail") || query.includes("gmail")) {
      const response = await axios.get(`${process.env.BASE_URL}/emails`);
      return res.status(200).json({ action: "email", data: response.data });
    }

    // Unknown intent
    return res.status(200).json({
      action: "unknown",
      message: "Sorry, I couldn't understand that.",
    });
  } catch (error) {
    console.error("Smart mode error:", error.message);
    return res
      .status(500)
      .json({ message: "Something went wrong in smart mode" });
  }
};
