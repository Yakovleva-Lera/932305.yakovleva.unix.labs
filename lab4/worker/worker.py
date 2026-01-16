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
    logger.info("Signal received. Finalizing current task...")
    running = False

signal.signal(signal.SIGTERM, shutdown_handler)
signal.signal(signal.SIGINT, shutdown_handler)

def main():
    consumer = None
    while consumer is None:
        try:
            consumer = KafkaConsumer(
                "image-processing",
                bootstrap_servers=KAFKA_BOOTSTRAP_SERVERS,
                group_id="image-workers",
                enable_auto_commit=False,
                value_deserializer=lambda x: json.loads(x.decode('utf-8')),
                request_timeout_ms=5000
            )
        except Exception:
            logger.info("Waiting for Kafka...")
            time.sleep(2)

    try:
        for message in consumer:
            if not running:
                break
            
            url = message.value.get("url")
            logger.info(f"Start processing: {url}")
            time.sleep(5)
            
            consumer.commit()
            logger.info(f"Committed: {url}")
            
            if not running:
                break
    finally:
        consumer.close()
        logger.info("Worker gracefully shut down.")

if __name__ == "__main__":
    main()
