import os
import signal
import time
import json
import logging
from kafka import KafkaConsumer

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("Worker")

running = True

def stop_handler(sig, frame):
    global running
    logger.info("Graceful shutdown initiated...")
    running = False

signal.signal(signal.SIGTERM, stop_handler)
signal.signal(signal.SIGINT, stop_handler)

def main():
    consumer = None
    while consumer is None and running:
        try:
            consumer = KafkaConsumer(
                "image-tasks",
                bootstrap_servers=os.getenv("KAFKA_BOOTSTRAP_SERVERS"),
                group_id="workers-group",
                enable_auto_commit=False,
                value_deserializer=lambda x: json.loads(x.decode('utf-8'))
            )
        except Exception:
            logger.info("Waiting for Kafka...")
            time.sleep(3)

    if not running:
        return

    logger.info("Worker started, waiting for messages...")
    
    try:
        for message in consumer:
            if not running:
                break
            
            data = message.value
            url = data.get("url")
            
            logger.info(f"PROCESSING START: {url}")
            time.sleep(10)
            
            consumer.commit()
            logger.info(f"PROCESSING DONE: {url}")
            
            if not running:
                break
    finally:
        if consumer:
            consumer.close()
        logger.info("Worker stopped cleanly.")

if __name__ == "__main__":
    main()
