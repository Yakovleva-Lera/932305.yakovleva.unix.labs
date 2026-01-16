import os
import signal
import time
import json
import logging
from kafka import KafkaConsumer

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

running = True

def handle_exit(sig, frame):
    global running
    logger.info("Shutdown signal received...")
    running = False

signal.signal(signal.SIGINT, handle_exit)
signal.signal(signal.SIGTERM, handle_exit)

def main():
    consumer = None
    while consumer is None:
        try:
            consumer = KafkaConsumer(
                "image-tasks",
                bootstrap_servers=os.getenv("KAFKA_BOOTSTRAP_SERVERS"),
                group_id="workers-group",
                enable_auto_commit=False,
                value_deserializer=lambda x: json.loads(x.decode('utf-8'))
            )
        except Exception:
            time.sleep(2)

    for msg in consumer:
        if not running:
            break
        
        logger.info(f"Start processing: {msg.value['url']}")
        time.sleep(10)
        
        consumer.commit()
        logger.info(f"Finished: {msg.value['url']}")
        
        if not running:
            break

    consumer.close()
    logger.info("Bye!")

if __name__ == "__main__":
    main()
