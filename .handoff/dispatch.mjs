// 4 枠を常に埋める coder ディスパッチャ。tasks.md が唯一の状態（再起動で続きから）。
import { spawn, spawnSync } from 'node:child_process';
import fs from 'node:fs';

const TASKS = 'specs/001-student-2d-engine/tasks.md';
const LOG = '.orchestrator/dispatch.log';
const FAILED = '.orchestrator/dispatch-failed.md';
const SLOTS = Number(process.env.SLOTS || 4);
const VERIFY_EVERY = 12;
const ROC = process.env.ROC;
if (!ROC) throw new Error('ROC env not set');

const log = (m) => fs.appendFileSync(LOG, `[${new Date().toISOString()}] ${m}\n`);

function parseUnits() {
  const lines = fs.readFileSync(TASKS, 'utf8').split(/\r?\n/);
  const units = [];
  let barrier = false;
  for (let i = 0; i < lines.length; i++) {
    const l = lines[i];
    if (/^##\s/.test(l)) { barrier = true; continue; }
    const m = l.match(/^- \[ \] (T\d+)/);
    if (!m) continue;
    const u = { ids: [m[1]], text: [l.slice(6)], barrier: barrier || !l.includes('[P]') };
    barrier = false;
    const n = lines[i + 1]?.match(/^- \[ \] (T\d+)/);
    if (n && /のテスト/.test(l) && !/のテスト/.test(lines[i + 1])) {
      u.ids.push(n[1]); u.text.push(lines[i + 1].slice(6)); i++;
    }
    units.push(u);
  }
  return units;
}

function markDone(ids) {
  let s = fs.readFileSync(TASKS, 'utf8');
  for (const id of ids) s = s.replace(new RegExp(`^- \\[ \\] ${id}\\b`, 'm'), `- [X] ${id}`);
  fs.writeFileSync(TASKS, s);
}

const RULES = `
Project: current directory (Sprout Engine, C++20, SFML). Follow specs/001-student-2d-engine/{spec,plan,tasks,data-model}.md, specs/001-student-2d-engine/contracts/, and existing code style (e.g. engine/include/sprout/math/Vector2.hpp, tests/unit/math/).
Rules:
- Create/edit only the files named in the task, plus the CMakeLists.txt that must register them. Other coders edit in parallel: re-read a CMakeLists.txt immediately before editing it and make a minimal insertion; never rewrite it whole.
- Do NOT edit tasks.md. Do NOT commit. Do NOT run cmake/ctest builds (a central build runs periodically); keep code compile-clean by careful reading.
- If a dependency the task needs does not exist yet, implement the minimum needed inside the task's files or report STATUS: blocked with the reason.
Done: all task files written, tests cover the described cases, registered in CMake. First line of answer: STATUS: done | blocked | failed.`;

function runCoder(prompt) {
  return new Promise((res) => {
    const p = spawn('node', [ROC, 'run', 'coder', prompt], { shell: false });
    let out = '';
    p.stdout.on('data', (d) => (out += d)); p.stderr.on('data', (d) => (out += d));
    p.on('close', (code) => {
      let result = '';
      const rf = out.match(/^result: (.+)$/m)?.[1]?.trim();
      if (rf && fs.existsSync(rf)) result = fs.readFileSync(rf, 'utf8');
      const turns = Number(out.match(/turns=(\d+)/)?.[1] || 0);
      const bad = /STATUS:\s*(blocked|fail|need-info|rejected)/i.test(result);
      res({ ok: code === 0 && turns > 0 && result.trim().length > 1 && !bad, out, result });
    });
  });
}

function build() {
  const cmds = [['cmake', ['--preset', 'vs2022-debug']], ['cmake', ['--build', '--preset', 'vs2022-debug']], ['ctest', ['--preset', 'vs2022-debug', '--output-on-failure']]];
  for (const [c, a] of cmds) {
    const r = spawnSync(c, a, { encoding: 'utf8', maxBuffer: 1 << 28 });
    if (r.status !== 0) return { ok: false, tail: ((r.stdout || '') + (r.stderr || '')).split('\n').slice(-80).join('\n') };
  }
  return { ok: true };
}

async function verifyAndFix() {
  for (let i = 0; i < 3; i++) {
    log('verify: build+test');
    const b = build();
    if (b.ok) { log('verify: OK'); return; }
    log(`verify: FAIL (attempt ${i + 1})`);
    await runCoder(`Fix the build/test failures below with minimal root-cause fixes (you MAY run cmake --build --preset vs2022-debug and ctest --preset vs2022-debug yourself; you are the only one building now). Do not weaken tests.\n\n${b.tail}\n${RULES}`);
  }
  log('verify: still failing after 3 fixes; continuing');
  fs.appendFileSync(FAILED, `- build still failing at ${new Date().toISOString()}\n`);
}

const attempts = new Map();
const running = new Set();
let sinceVerify = 0;

async function main() {
  log(`start slots=${SLOTS}`);
  for (;;) {
    const units = parseUnits().filter((u) => !running.has(u.ids[0]) && (attempts.get(u.ids[0]) || 0) < 3);
    if (!units.length && !running.size) break;
    const next = units[0];
    const mustWait = next && (next.barrier || sinceVerify >= VERIFY_EVERY) && running.size > 0;
    if (next && !mustWait && running.size < SLOTS) {
      if (next.barrier || sinceVerify >= VERIFY_EVERY) { await verifyAndFix(); sinceVerify = 0; }
      const key = next.ids[0];
      running.add(key);
      attempts.set(key, (attempts.get(key) || 0) + 1);
      log(`dispatch ${next.ids.join('+')} (try ${attempts.get(key)}) running=${running.size}`);
      runCoder(`Task ${next.ids.join(' + ')}:\n${next.text.join('\n')}\n${RULES}`).then((r) => {
        running.delete(key);
        if (r.ok) { markDone(next.ids); sinceVerify++; log(`done ${next.ids.join('+')}`); }
        else {
          log(`fail ${next.ids.join('+')}: ${r.result.split('\n')[0] || r.out.slice(-200)}`);
          if (attempts.get(key) >= 3) fs.appendFileSync(FAILED, `- ${next.ids.join('+')}: ${(r.result || r.out).slice(0, 400).replace(/\n/g, ' ')}\n`);
        }
        wake();
      });
      continue; // すぐ次の空き枠を埋める
    }
    await new Promise((r) => (wake = r));
  }
  await verifyAndFix();
  log('all done');
}
let wake = () => {};
main();
