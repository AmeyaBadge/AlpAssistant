import express from "express";
import { fetchNews } from "../controllers/esp.controller.js";
import { fetchWeather } from "../controllers/esp.controller.js";
import { protectRoute } from "../middleware/auth.middleware.js";
import { smartHandler } from "../controllers/smart.controller.js";
import {
  getEmails,
  gmailCallback,
  loginToGmail,
} from "../controllers/gmail.controller.js";

const router = express.Router();

router.get("/weather", protectRoute, fetchWeather);
router.get("/news", protectRoute, fetchNews); //       /news?category=technology&country=us
router.get("/smart", protectRoute, smartHandler); //       /smart?query=temperature+in+new+york

router.get("/gmail/login", loginToGmail);
router.get("/gmail/callback", gmailCallback);
router.get("/gmail/emails", getEmails);

export default router;
