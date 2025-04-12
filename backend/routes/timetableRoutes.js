const express = require("express");
const runCppWithJson = require("../utils/runCpp");

const router = express.Router();

router.post("/generate-timetable", async (req, res) => {
    try {
        const inputJson = req.body; // JSON input from Postman
        const result = await runCppWithJson(inputJson); // Run C++ generator
        res.json(result); // Send JSON response back
    } catch (error) {
        res.status(500).json({ error: error.toString() });
    }
});

module.exports = router;
