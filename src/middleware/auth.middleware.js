export const protectRoute = async (req, res, next) => {
  try {
    // const api = req.query.api;
    // if (api !== process.env.API_KEY)
    //   return res.status(401).json({ message: "Unauthorised Access" });

    next(); // used to call the next function in the PUT method
  } catch (error) {
    res.status(500).json({ message: "Internal Server Error" });
    console.log("Error in protectRoute middleware: ", error);
  }
};
