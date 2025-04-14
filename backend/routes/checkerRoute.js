const express = require("express");
const router = express.Router();
const runCpp2WithJson = require("../utils/runCpp2");

router.post("/", async (req, res) => {
    try {
        const inputJson = req.body;
        const result = await runCpp2WithJson(inputJson);
        // console.log(result);
        res.json(result);
    } catch (error) {
        console.error("Error running checker:", error);
        res.status(500).json({ error: "Checker execution failed" });
    }
});

module.exports = router;
