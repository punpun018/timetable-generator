const express = require("express");
const multer = require("multer");
const { runPython } = require("../utils/runPython");

const router = express.Router();

// Configure Multer for file uploads
const storage = multer.memoryStorage();
const upload = multer({ storage });

router.post("/", upload.single("file"), async (req, res) => {
    console.log(`Received request with file: ${req.file ? req.file.originalname : 'No file'}`);

    if (!req.file) return res.status(400).json({ error: "No file uploaded" });

    try {
        // Run Python script to parse the Excel file
        const adjacencyGraph = await runPython(req.file.buffer);
        console.log(`Successfully processed file: ${req.file.originalname}`);

        res.json({ adjacencyGraph });
    } catch (error) {
        res.status(500).json({ error: "Error processing file", details: error.toString() });

    }
});

module.exports = router;
