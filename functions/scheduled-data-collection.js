import Particle from 'particle:core';

export default function job({ functionInfo, trigger, scheduled }) {
  Particle.publish('READ-DATA', null, { productId: MY_PRODUCT_0 });
  Particle.publish('READ-DATA', null, { productId: MY_PRODUCT_1 });
}