const express = require("express");
const bodyParser = require("body-parser");
const cors = require("cors");
const app = express();
const slotSuggestionRoutes = require('./routes/slotSuggestionRoute');
const checkerRoutes = require("./routes/checkerRoute");

app.use(cors({
    origin: ["https://timetable-generator-new-olive.vercel.app", "http://localhost:5173"], // Allow both origins
    methods: ['GET', 'PUT', 'POST', 'FETCH', 'DELETE'],
    credentials: true,            //access-control-allow-credentials:true
    optionSuccessStatus: 200
}));
// Increase JSON payload limit
app.use(bodyParser.json({ limit: "50mb" })); // Increase to 50MB
app.use(bodyParser.urlencoded({ limit: "50mb", extended: true })); // Increase for URL-encoded data


app.use(express.json());

app.use("/upload", require("./routes/uploadRoutes"));
app.use("/timetable", require("./routes/validateRoutes"));
const excelRoutes = require("./routes/excelRoutes");
app.use("/excel", excelRoutes);
app.use('/api/slots', slotSuggestionRoutes);
app.use("/api/checker", checkerRoutes);
app.get("/", (req, res) => {
    res.send("Backend is running ✅");
});

app.get('/health', (req, res) => {
    res.send("SERVER ON");
});
const PORT = 5000;
app.listen(PORT, () => console.log(`Server running on port ${PORT}`));

module.exports = app;