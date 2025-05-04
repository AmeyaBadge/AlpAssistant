/* 
    400 - Bad Request from Client
    401 - Invalid Credentials from Client
    500 - Internal server error
    404 - Not Found
    201 - Succesfully created new Resourcef
*/
import axios from "axios";

//Weather API
export const fetchWeather = async (req, res) => {
  try {
    const city = req.query.city || "Pune";
    const apiKey = process.env.WEATHER_API_KEY;

    if (!apiKey) {
      return res
        .status(500)
        .json({ message: "Weather API key not set in environment" });
    }

    const weatherURL = `https://api.openweathermap.org/data/2.5/weather?q=${city}&appid=${apiKey}&units=metric`;

    const response = await axios.get(weatherURL);
    const data = response.data;

    const weatherReport = {
      city: data.name,
      description: data.weather[0].description,
      temperature: data.main.temp,
      humidity: data.main.humidity,
      windSpeed: data.wind.speed,
    };

    res.status(200).json({
      message: `Weather in ${weatherReport.city}: ${weatherReport.description}, Temp: ${weatherReport.temperature}°C`,
      data: weatherReport,
    });
  } catch (error) {
    console.error("Error fetching weather: ", error.message);
    if (error.response?.status === 404) {
      res.status(404).json({ message: "City not found" });
    } else {
      res.status(500).json({ message: "Internal Server Error" });
    }
  }
};

//News API
export const fetchNews = async (req, res) => {
  const category = req.query.category.trim() || ""; // default category
  const country = req.query.country || "in"; // default to India
  // const q = req.query.country || ""; // blank
  console.log("Category:", category);
  try {
    console.log(
      `https://newsdata.io/api/1/latest?apikey=${process.env.NEWS_API}&country=${country}&category=${category}&language=en`
    );
    const response = await axios.get(
      `https://newsdata.io/api/1/latest?apikey=${process.env.NEWS_API}&country=${country}&category=${category}&language=en`
    );
    const articles = response.data.results.map((article) => ({
      title: article.title,
      source: article.source_name,
      url: article.link,
      description:
        article.description != null
          ? article.description.substring(0, 200) + "..."
          : "",
    }));

    res.status(200).json({
      message: "Top news fetched successfully",
      articles,
    });
  } catch (error) {
    console.error("Error fetching news:", error.message);
    res.status(500).json({ message: "Failed to fetch news" });
  }
};
