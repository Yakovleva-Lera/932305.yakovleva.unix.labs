import os
import json
import time
from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
from kafka import KafkaProducer

app = FastAPI()
KAFKA_BOOTSTRAP_SERVERS = os.getenv("KAFKA_BOOTSTRAP_SERVERS", "localhost:9092")
producer = None

def get_producer():
    global producer
    if producer is None:
        try:
            producer = KafkaProducer(
                bootstrap_servers=KAFKA_BOOTSTRAP_SERVERS,
                value_serializer=lambda v: json.dumps(v).encode('utf-8'),
                retries=5
            )
        except Exception:
            return None
    return producer

class TaskRequest(BaseModel):
    url: str

@app.post("/process")
async def process(request: TaskRequest):
    p = get_producer()
    if not p:
        raise HTTPException(status_code=503, detail="Kafka is not ready")
    p.send("image-tasks", {"url": request.url})
    return {"status": "accepted"}
