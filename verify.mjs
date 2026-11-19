import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { createHash } from 'node:crypto';

const binary = readFileSync(new URL('./combat.wasm', import.meta.url));
const { instance: { exports: c } } = await WebAssembly.instantiate(binary);
const [NONE, OUTSIDE, INTERRUPTED, INVULNERABLE, BLOCKED, PARRIED, DUPLICATE, HIT] = Array.from({ length: 8 }, (_, i) => i);
const names = ['NONE', 'OUTSIDE_WINDOW', 'INTERRUPTED', 'INVULNERABLE', 'BLOCKED', 'PARRIED', 'DUPLICATE', 'HIT'];
let count = 0;

function check(name, run) {
  c.reset_match();
  assert.equal(c.start_attack(17, 104, 107), 1);
  run();
  count += 1;
  console.log(`PASS ${name}`);
}

const r = (tick, target = 0, defense = 0, attack = 17) => c.resolve_candidate(attack, tick, target, defense);
console.log(`Node ${process.version}`);
console.log(`combat.wasm SHA-256 ${createHash('sha256').update(binary).digest('hex')}`);

check('window boundaries, invulnerability and exact-once damage', () => {
  const rows = [[103, 0, OUTSIDE], [104, 1, INVULNERABLE], [105, 0, HIT], [106, 0, DUPLICATE], [107, 0, OUTSIDE]];
  for (const [tick, defense, expected] of rows) {
    assert.equal(r(tick, 0, defense), expected);
    console.log(`  tick=${tick} attack=A17 target=E04 ${names[expected]} hp=${c.target_hp(0)} commits=${c.damage_commits()}`);
  }
  assert.equal(c.damage_commits(), 1);
  assert.equal(c.target_hp(0), 75);
});
check('same-frame duplicate contact', () => {
  assert.equal(r(104), HIT);
  assert.equal(r(104), DUPLICATE);
  assert.equal(c.damage_commits(), 1);
});
check('one swing hits each of two targets once', () => {
  assert.equal(r(104, 1), HIT);
  assert.equal(r(104, 0), HIT);
  assert.equal(r(105, 1), DUPLICATE);
  assert.equal(c.damage_commits(), 2);
  assert.deepEqual([c.target_hp(0), c.target_hp(1)], [75, 75]);
});
check('guard and parry leave hit quota intact', () => {
  assert.equal(r(104, 0, 2), BLOCKED);
  assert.equal(r(105, 0, 3), PARRIED);
  assert.equal(c.damage_commits(), 0);
  assert.equal(r(106), HIT);
});
check('interrupt and stale callback are rejected', () => {
  assert.equal(c.interrupt_attack(17), 1);
  assert.equal(r(104), INTERRUPTED);
  assert.equal(c.start_attack(18, 105, 108), 1);
  assert.equal(r(105), INTERRUPTED);
  assert.equal(r(105, 0, 0, 18), HIT);
});
check('new instance may damage the same target again', () => {
  assert.equal(r(104), HIT);
  assert.equal(c.start_attack(18, 110, 113), 1);
  assert.equal(r(110, 0, 0, 18), HIT);
  assert.equal(c.target_hp(0), 50);
});
check('sweep detects a crossing missed by both endpoint overlaps', () => {
  assert.equal(c.sweep_contact(0, 0, 0, 0, 5, 0, 1), 0);
  assert.equal(c.sweep_contact(10, 0, 10, 0, 5, 0, 1), 0);
  assert.equal(c.sweep_contact(0, 0, 10, 0, 5, 0, 1), 1);
  assert.equal(c.sweep_contact(0, 0, 10, 0, 5, 2, 1), 0);
});
check('invalid candidate and reused instance leave HP intact', () => {
  assert.equal(r(104, 4), NONE);
  assert.equal(r(104, 0, 99), NONE);
  assert.equal(c.start_attack(17, 104, 107), 0);
  assert.equal(c.start_attack(18, 107, 107), 0);
  assert.equal(c.target_hp(0), 100);
  assert.equal(c.damage_commits(), 0);
});
console.log(`${count} / ${count} scenarios passed`);
