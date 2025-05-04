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

    // Check for news category
    else if (
      query.includes("news") ||
      doc.has("headline") ||
      doc.has("updates")
    ) {
      // Check for noun and adjective matches
      let category =
        (doc.match("#Noun").last().text() === "news"
          ? "technology"
          : doc.match("#Noun").last().text()) || "technology";

      // Handling sports with adjectives
      if (doc.match("#Adjective").has("sports")) {
        category = "sports";
      }

      // Additional checks for specific categories
      else if (doc.match("#Noun").has("business")) {
        category = "business";
      } else if (doc.match("#Noun").has("entertainment")) {
        category = "entertainment";
      } else if (doc.match("#Noun").has("general")) {
        category = "general";
      } else if (doc.match("#Noun").has("health")) {
        category = "health";
      } else if (doc.match("#Noun").has("science")) {
        category = "science";
      }

      // Ensure valid category: Business, Entertainment, General, Health, Science, Sports, Technology
      const validCategories = [
        "business",
        "entertainment",
        "general",
        "health",
        "science",
        "sports",
        "technology",
      ];
      if (!validCategories.includes(category)) {
        category = "technology"; // Default to technology if not valid
      }

      console.log("Category detected:", category);

      // Fetch news based on the detected category
      try {
        const response = await axios.get(
          `${process.env.BASE_URL}/news?category=${category}`
        );
        return res.status(200).json({ action: "news", data: response.data });
      } catch (error) {
        console.error("Error fetching news:", error);
        return res.status(500).json({ error: "Failed to fetch news" });
      }
    }

    // Check for email
    if (
      query.includes("email") ||
      doc.has("mail") ||
      doc.has("mails") ||
      query.includes("gmail")
    ) {
      const response = await axios.get(`${process.env.BASE_URL}/gmail/emails`);
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
