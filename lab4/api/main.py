import os
import json
import time
from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
from kafka import KafkaProducer

app = FastAPI()
KAFKA_SERVERS = os.getenv("KAFKA_BOOTSTRAP_SERVERS", "localhost:9092")
_producer = None

def get_producer():
    global _producer
    if _producer is None:
        try:
            _producer = KafkaProducer(
                bootstrap_servers=KAFKA_SERVERS,
                value_serializer=lambda v: json.dumps(v).encode('utf-8'),
                request_timeout_ms=5000
            )
        except Exception:
            return None
    return _producer

class Task(BaseModel):
    url: str

@app.post("/process")
async def process(task: Task):
    producer = get_producer()
    if not producer:
        raise HTTPException(status_code=503, detail="Kafka not ready")
    
    try:
        producer.send("image-tasks", {"url": task.url})
        return {"status": "accepted", "url": task.url}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)
