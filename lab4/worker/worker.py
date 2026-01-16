import os
import signal
import time
import json
import logging
from kafka import KafkaConsumer

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

KAFKA_BOOTSTRAP_SERVERS = os.getenv("KAFKA_BOOTSTRAP_SERVERS", "localhost:9092")
running = True

def shutdown_handler(signum, frame):
    global running
    logger.info("Shutdown signal received.")
    running = False

signal.signal(signal.SIGTERM, shutdown_handler)
signal.signal(signal.SIGINT, shutdown_handler)

def main():
    consumer = KafkaConsumer(
        "image-processing",
        bootstrap_servers=KAFKA_BOOTSTRAP_SERVERS,
        group_id="image-workers",
        enable_auto_commit=False,
        value_deserializer=lambda x: json.loads(x.decode('utf-8'))
    )

    try:
        for message in consumer:
            if not running:
                break
            
            data = message.value
            url = data.get("url")
            
            logger.info(f"Processing: {url}")
            time.sleep(10) 
            
            consumer.commit()
            logger.info(f"Done and committed: {url}")
            
            if not running:
                break
    finally:
        consumer.close()
        logger.info("Worker stopped.")

if name == "__main__":
    main()
