const express = require("express");
const bodyParser = require("body-parser");
const cors = require("cors");
const app = express();
const slotSuggestionRoutes = require('./routes/slotSuggestionRoute');

// Increase JSON payload limit
app.use(bodyParser.json({ limit: "50mb" })); // Increase to 50MB
app.use(bodyParser.urlencoded({ limit: "50mb", extended: true })); // Increase for URL-encoded data

app.use(cors());
app.use(express.json());

app.use("/upload", require("./routes/uploadRoutes"));
app.use("/timetable", require("./routes/validateRoutes"));
app.use("/excel", require("./routes/excelRoutes"));
app.use('/api/slots', slotSuggestionRoutes);

const PORT = 5000;
app.listen(PORT, () => console.log(`Server running on port ${PORT}`));
