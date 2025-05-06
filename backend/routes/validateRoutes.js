const express = require("express");
const runCpp3 = require("../utils/runCpp3");

const router = express.Router();

router.post("/validate-swap", async (req, res) => {
    try {
        const prevjson = req.body;

        if (!prevjson || !prevjson.Swap) {
            return res.status(400).json({ error: "Invalid input: 'prevjson' or 'Swap' missing." });
        }

        // Call checker2.exe with prevjson
        const newjson = await runCpp3(prevjson);

        if (!newjson) {
            return res.status(500).json({ error: "Swap validation failed or returned empty result." });
        }


        delete newjson.Swap;

        res.json(newjson);
    } catch (err) {
        console.error("Error in /validate-swap:", err);
        res.status(500).json({ error: "Internal server error", details: err.toString() });
    }
});

module.exports = router;
