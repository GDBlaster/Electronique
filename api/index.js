const express = require("express");
const db = require('./db.json');


const app = express();
app.use(express.json());
const PORT = process.env.PORT || 3000;

app.listen(PORT, () => {
    console.log("server listening on port", PORT);
});

app.get("/ping", (request, response) => {
    var status = {
        "Ping": "Pong"
    }

    response.send(status);
});

app.get("/check", (request, response) => {
    console.log(request.body);
    if (db.tokens.includes(request.body.token)) {
        if (db.badges[request.body.id] != undefined) {
            var out = {
                "permission" : db.badges[request.body.id],
            }
            response.send(out)
        } else {
            response.status(404).json({error : "Not Found"});
        };
    } else {
        response.status(403).json({ error : "Forbiden"});
    };

});
