import express from "express";
import dotenv from "dotenv";
import espRoutes from "./routes/esp.route.js";
import cors from "cors";

const app = express();
dotenv.config({ path: `.env.${process.env.NODE_ENV || "development"}` });

app.use(express.json());

app.use(
  cors({
    origin: "http://localhost:3000",
    credentials: true, // Allow sending of auth cookies and headers
  })
);

app.use("/api", espRoutes);

const PORT = process.env.PORT;

app.listen(PORT, () => {
  console.log(`Server started on PORT:${PORT}`);
});
