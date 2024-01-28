import Particle from 'particle:core';

export default function reformat({ event }) {
  let data;
  try {
	  data = JSON.parse(event.eventData);
  } catch (err) {
    console.error("Invalid JSON", event.eventData);
    throw err;
  }

  const sensorLedger = Particle.ledger('sensor-data');
  const currentData = sensorLedger.get();

  let newValue;

  if (data.type === 'temp') {
    newValue = {
        temp: {
            ts: data.ts,
            tempF: data.tempF
        }
    };
  } else if (data.type === 'imu') {
    newValue = {
        imu: {
            ts: data.ts,
            x: data.x,
            y: data.y,
            z: data.z
        }
    };
  } else {
    return;
  }

  sensorLedger.set(newValue, Particle.MERGE);
}