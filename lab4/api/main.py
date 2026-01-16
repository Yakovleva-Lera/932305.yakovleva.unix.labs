from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
from kafka import KafkaProducer
import os
import json
import time

app = FastAPI()

producer = None
KAFKA_BOOTSTRAP_SERVERS = os.getenv("KAFKA_BOOTSTRAP_SERVERS", "localhost:9092")

def get_producer():
    global producer
    if producer is None:
        for _ in range(10):
            try:
                producer = KafkaProducer(
                    bootstrap_servers=KAFKA_BOOTSTRAP_SERVERS,
                    value_serializer=lambda v: json.dumps(v).encode('utf-8')
                )
                break
            except Exception:
                time.sleep(3)
    return producer

class ImageRequest(BaseModel):
    url: str

@app.post("/process")
async def process_image(request: ImageRequest):
    p = get_producer()
    if not p:
        raise HTTPException(status_code=503, detail="Kafka not ready")
    try:
        p.send("image-processing", {"url": request.url})
        return {"status": "accepted"}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))
