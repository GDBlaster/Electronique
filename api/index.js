import express from "express";

const app = express();
app.use(express.json);
const PORT = process.env.PORT || 3000;

app.listen(PORT,() => {
    console.log("server listening on port", PORT);
});

app.get("/ping" , (request , response) => {
    var status = {
        "Ping" : "Pong"
    }

    response.send(status);
});

