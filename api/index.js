const express = require("express");
const db = require('./db.json');
const LOG_FILE = "logs.json";

const app = express();
app.use(express.json());
const PORT = process.env.PORT || 3000;

function logToFile(entry) {
    let logs = { entries: [] };
    if (fs.existsSync(LOG_FILE)) {
        try {
            logs = JSON.parse(fs.readFileSync(LOG_FILE, "utf8"));
        } catch (error) {
            console.error("Error parsing logs.json, resetting log file.");
        }
    }
    logs.entries.push(entry);
    fs.writeFileSync(LOG_FILE, JSON.stringify(logs, null, 2));
}

app.listen(PORT, () => {
    console.log("server listening on port", PORT);
});

app.get("/ping", (request, response) => {
    var status = {
        "Ping": "Pong"
    }

    response.send(status);
});

app.post("/check", (request, response) => {
    const { token, id } = request.body;
    const timestamp = new Date().toISOString();
    let userLevel = "none";
    let status;

    if (db.tokens.includes(token)) {
        if (db.badges[id] !== undefined) {
            userLevel = db.badges[id];
            response.send({ "permission": userLevel });
            status = 200;
        } else {
            response.status(404).json({ error: "Not Found" });
            status = 404;
        }
    } else {
        response.status(403).json({ error: "Forbidden" });
        status = 403;
    }

    logToFile({ token, id, timestamp, userLevel, status });
});

app.get("/logs", (req, res) => {
    fs.readFile(LOG_FILE, "utf8", (err, data) => {
        if (err) {
            res.status(500).send("Error reading logs");
        } else {
            res.setHeader("Content-Type", "application/json");
            res.send(data);
        }
    });
});
