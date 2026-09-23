(() => {
  'use strict';

  const slides = [...document.querySelectorAll('.slide')];
  const dots = document.querySelector('#progress-dots');
  const state = { index: 0, frame: 0, emergency: false, calibrated: false, x: 0, y: 0, z: 10, started: Date.now(), telemetry: { battery: 86, tilt: 2.4, temp: 31.8, pressure: 1008, humidity: 54, gas: .18 } };
  const $ = (id) => document.getElementById(id);
  const clamp = (value, min, max) => Math.max(min, Math.min(max, value));

  slides.forEach((slide, index) => {
    const dot = document.createElement('button');
    dot.type = 'button';
    dot.setAttribute('aria-label', `Ir para o slide ${index + 1}`);
    dot.addEventListener('click', () => goTo(index));
    dots.appendChild(dot);
  });

  function goTo(index) {
    state.index = (index + slides.length) % slides.length;
    slides.forEach((slide, slideIndex) => {
      const active = slideIndex === state.index;
      slide.classList.toggle('active', active);
      slide.setAttribute('aria-hidden', String(!active));
    });
    [...dots.children].forEach((dot, dotIndex) => dot.classList.toggle('active', dotIndex === state.index));
    $('slide-count').textContent = `${String(state.index + 1).padStart(2, '0')} / ${String(slides.length).padStart(2, '0')}`;
    history.replaceState(null, '', `#${slides[state.index].id}`);
  }

  function goToId(id) {
    const index = slides.findIndex((slide) => slide.id === id);
    if (index >= 0) goTo(index);
  }

  function toast(message) {
    const element = $('toast');
    element.textContent = message;
    element.classList.add('show');
    clearTimeout(toast.timer);
    toast.timer = setTimeout(() => element.classList.remove('show'), 2400);
  }

  function log(message, tag = 'SYS') {
    const terminal = $('lab-terminal');
    const line = document.createElement('span');
    line.innerHTML = `<b>${new Date().toLocaleTimeString('pt-BR', { hour12: false })}</b> [${tag}] ${message}`;
    terminal.appendChild(line);
    while (terminal.children.length > 7) terminal.removeChild(terminal.firstChild);
  }

  function emit(event, payload) {
    window.dispatchEvent(new CustomEvent(`armtank:${event}`, { detail: payload }));
  }

  function drawPixelRover(canvas, detail = false) {
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    const width = canvas.width;
    const height = canvas.height;
    const ratio = width / 900;
    ctx.clearRect(0, 0, width, height);
    ctx.fillStyle = detail ? '#aebe82' : '#162219';
    ctx.fillRect(0, 0, width, height);
    ctx.strokeStyle = detail ? 'rgba(43,68,44,.25)' : 'rgba(200,243,106,.08)';
    ctx.lineWidth = 1;
    for (let x = 0; x < width; x += 45 * ratio) { ctx.beginPath(); ctx.moveTo(x, 0); ctx.lineTo(x, height); ctx.stroke(); }
    for (let y = 0; y < height; y += 45 * ratio) { ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(width, y); ctx.stroke(); }
    const bob = Math.sin(state.frame / 32) * 4;
    const bx = width * .5 + state.x * 75 * ratio;
    const by = height * .61 + state.y * 46 * ratio + bob;
    ctx.save();
    ctx.translate(bx, by);
    ctx.rotate(state.x * .05);
    ctx.imageSmoothingEnabled = false;
    ctx.fillStyle = detail ? '#243b28' : '#324d31';
    ctx.fillRect(-128 * ratio, -27 * ratio, 256 * ratio, 74 * ratio);
    ctx.fillStyle = detail ? '#1a2c20' : '#203622';
    ctx.fillRect(-141 * ratio, -14 * ratio, 18 * ratio, 61 * ratio);
    ctx.fillRect(123 * ratio, -14 * ratio, 18 * ratio, 61 * ratio);
    ctx.fillStyle = detail ? '#b3d85f' : '#769853';
    ctx.fillRect(-104 * ratio, -19 * ratio, 208 * ratio, 58 * ratio);
    ctx.fillStyle = detail ? '#658451' : '#4c7147';
    ctx.fillRect(-82 * ratio, -8 * ratio, 164 * ratio, 31 * ratio);
    ctx.fillStyle = detail ? '#d4eb83' : '#9dbd62';
    ctx.fillRect(-57 * ratio, -4 * ratio, 83 * ratio, 17 * ratio);
    ctx.fillStyle = '#1a2c20';
    for (let wheel = -112; wheel < 120; wheel += 23) ctx.fillRect(wheel * ratio, 48 * ratio, 15 * ratio, 10 * ratio);
    ctx.fillStyle = '#c8f36a';
    ctx.fillRect(-5 * ratio, 48 * ratio, 15 * ratio, 7 * ratio);
    ctx.strokeStyle = detail ? '#d6ee8a' : '#b1d36b';
    ctx.lineWidth = 13 * ratio;
    ctx.lineCap = 'square';
    ctx.beginPath();
    ctx.moveTo(18 * ratio, -20 * ratio);
    ctx.lineTo(23 * ratio, -72 * ratio);
    ctx.lineTo(85 * ratio, -99 * ratio);
    ctx.lineTo(120 * ratio, -78 * ratio);
    ctx.stroke();
    ctx.fillStyle = '#f4bb61';
    ctx.fillRect(116 * ratio, -87 * ratio, 19 * ratio, 11 * ratio);
    ctx.fillRect(132 * ratio, -80 * ratio, 12 * ratio, 7 * ratio);
    ctx.restore();
    if (detail) {
      ctx.fillStyle = '#273e28';
      ctx.font = `${11 * ratio}px monospace`;
      ctx.fillText('Z 10.0 cm', 34 * ratio, 42 * ratio);
      ctx.fillText('X +0.42 m  Y -0.18 m', 34 * ratio, 59 * ratio);
      ctx.fillStyle = '#f4bb61';
      ctx.fillText(state.emergency ? 'SAFE MODE' : 'TARGET // Z-LOCK', width - 185 * ratio, height - 30 * ratio);
    }
  }

  function drawKinematics() {
    const canvas = $('kinematics-canvas');
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    const w = canvas.width;
    const h = canvas.height;
    ctx.clearRect(0, 0, w, h);
    ctx.fillStyle = '#142019';
    ctx.fillRect(0, 0, w, h);
    ctx.strokeStyle = 'rgba(200,243,106,.1)';
    for (let x = 0; x < w; x += 48) { ctx.beginPath(); ctx.moveTo(x, 0); ctx.lineTo(x, h); ctx.stroke(); }
    for (let y = 0; y < h; y += 48) { ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(w, y); ctx.stroke(); }
    const baseX = 170 + Math.sin(state.frame / 50) * 20;
    const baseY = 310;
    const targetX = 515 + Math.sin(state.frame / 64) * 25;
    const targetY = 120;
    ctx.strokeStyle = '#c8f36a';
    ctx.lineWidth = 14;
    ctx.lineCap = 'square';
    ctx.beginPath();
    ctx.moveTo(baseX, baseY);
    ctx.lineTo(baseX, baseY - 90);
    ctx.lineTo(340, 175);
    ctx.lineTo(targetX, targetY);
    ctx.stroke();
    ctx.fillStyle = '#f4bb61';
    ctx.fillRect(targetX - 10, targetY - 7, 24, 14);
    ctx.strokeStyle = 'rgba(114,224,202,.65)';
    ctx.setLineDash([7, 8]);
    ctx.beginPath(); ctx.moveTo(targetX, targetY); ctx.lineTo(targetX, baseY); ctx.stroke();
    ctx.setLineDash([]);
    ctx.fillStyle = '#72e0ca';
    ctx.fillRect(targetX - 4, baseY - 3, 8, 8);
    ctx.font = '13px monospace';
    ctx.fillText('Z = 10 cm', targetX + 18, (targetY + baseY) / 2);
    $('pose-x').textContent = `+${(0.42 + Math.sin(state.frame / 90) * .04).toFixed(2)}`;
    $('pose-y').textContent = `-${(0.18 + Math.cos(state.frame / 80) * .03).toFixed(2)}`;
    $('pose-rz').textContent = `+${(12.4 + Math.sin(state.frame / 60) * 1.4).toFixed(1)}`;
  }

  function drawVideo() {
    const canvas = $('video-canvas');
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    const w = canvas.width;
    const h = canvas.height;
    ctx.fillStyle = '#1f3326';
    ctx.fillRect(0, 0, w, h);
    ctx.fillStyle = '#6e8c58';
    ctx.beginPath(); ctx.moveTo(0, h * .72); ctx.lineTo(w * .42, h * .34); ctx.lineTo(w, h * .57); ctx.lineTo(w, h); ctx.lineTo(0, h); ctx.fill();
    ctx.fillStyle = '#293f2b';
    ctx.beginPath(); ctx.moveTo(0, h * .45); ctx.lineTo(w * .25, h * .25); ctx.lineTo(w * .5, h * .38); ctx.lineTo(w * .75, h * .17); ctx.lineTo(w, h * .3); ctx.lineTo(w, 0); ctx.lineTo(0, 0); ctx.fill();
    ctx.strokeStyle = 'rgba(200,243,106,.3)';
    ctx.strokeRect(w * .35, h * .25, w * .3, h * .42);
    ctx.fillStyle = '#c8f36a';
    ctx.font = '12px monospace';
    ctx.fillText(`CAM-A / FRAME ${String(state.frame).padStart(5, '0')}`, 20, h - 20);
  }

  function updateTelemetry() {
    const t = state.telemetry;
    const seconds = (Date.now() - state.started) / 1000;
    t.battery = clamp(86 - seconds * .012, 0, 100);
    t.tilt = 2.4 + Math.sin(seconds * .7) * 1.3;
    t.temp = 31.8 + Math.sin(seconds * .22) * 1.1;
    t.pressure = 1008 + Math.sin(seconds * .1) * 3;
    t.humidity = 54 + Math.sin(seconds * .16) * 3;
    t.gas = .18 + Math.abs(Math.sin(seconds * .13)) * .05;
    const values = { battery: `${Math.round(t.battery)}%`, tilt: `${t.tilt >= 0 ? '+' : ''}${t.tilt.toFixed(1)}°`, temp: `${t.temp.toFixed(1)}°C`, pressure: `${Math.round(t.pressure)} hPa`, humidity: `${Math.round(t.humidity)}% RH`, gas: `${t.gas.toFixed(2)} ppm` };
    Object.entries(values).forEach(([key, value]) => { if ($(`sensor-${key}`)) $(`sensor-${key}`).textContent = value; });
    [['battery', t.battery], ['tilt', Math.abs(t.tilt) * 12], ['temp', t.temp * 2], ['pressure', 48], ['humidity', t.humidity], ['gas', t.gas * 220]].forEach(([key, value]) => { if ($(`bar-${key}`)) $(`bar-${key}`).style.width = `${clamp(value, 5, 100)}%`; });
    $('lab-frame').textContent = String(state.frame).padStart(5, '0');
    $('lab-connection').textContent = `${Math.round(16 + Math.sin(seconds) * 3)} ms`;
    $('lab-time').textContent = new Date(seconds * 1000).toISOString().slice(14, 19);
    $('hero-latency').textContent = String(Math.round(16 + Math.sin(seconds) * 3)).padStart(3, '0');
  }

  function setJoystick(x, y) {
    state.x = clamp(x, -1, 1);
    state.y = clamp(y, -1, 1);
    const knob = document.querySelector('.joystick-knob');
    if (knob) knob.style.transform = `translate(${state.x * 48}px, ${state.y * 48}px)`;
    $('lab-velocity').textContent = String(Math.round(-state.y * 100)).padStart(3, '0');
    $('lab-direction').textContent = String(Math.round(state.x * 100)).padStart(3, '0');
    emit('command', { type: 'drive', x: state.x, y: state.y });
  }

  function bindJoystick() {
    const joystick = $('joystick');
    if (!joystick) return;
    let active = false;
    const update = (event) => { const rect = joystick.getBoundingClientRect(); setJoystick((event.clientX - rect.left - rect.width / 2) / (rect.width / 2), (event.clientY - rect.top - rect.height / 2) / (rect.height / 2)); };
    joystick.addEventListener('pointerdown', (event) => { active = true; joystick.setPointerCapture(event.pointerId); update(event); });
    joystick.addEventListener('pointermove', (event) => { if (active) update(event); });
    joystick.addEventListener('pointerup', () => { active = false; setJoystick(0, 0); });
    joystick.addEventListener('keydown', (event) => { const keys = { ArrowUp: [0, -1], ArrowDown: [0, 1], ArrowLeft: [-1, 0], ArrowRight: [1, 0] }; if (keys[event.key]) { event.preventDefault(); setJoystick(...keys[event.key]); } });
  }

  function calibrate() {
    $('lab-log-state').textContent = 'CALIBRANDO';
    log('Grade A4 detectada. Lendo referência X/Y.', 'CAL');
    toast('Calibração iniciada: grade + IMU + atuadores');
    setTimeout(() => { state.calibrated = true; $('lab-log-state').textContent = 'PROFILE READY'; log('Perfil salvo: 6 servos + 2 motores / Z-lock validado.', 'CAL'); toast('Perfil de missão validado'); emit('calibrated', { axes: 6, motors: 2, grid: '10x10cm' }); }, 1800);
  }

  function emergencyStop() {
    state.emergency = !state.emergency;
    $('lab-mode').textContent = state.emergency ? 'SAFE MODE' : 'TELEOPERAÇÃO';
    $('lab-log-state').textContent = state.emergency ? 'ACTUATORS OFF' : 'SYSTEM READY';
    $('lab-stop').textContent = state.emergency ? 'LIBERAR' : 'STOP';
    log(state.emergency ? 'Atuadores desarmados. Pose segura aplicada.' : 'Controle liberado pelo operador.', state.emergency ? 'SAFE' : 'SYS');
    toast(state.emergency ? 'PARADA DE EMERGÊNCIA ATIVA' : 'Controle liberado');
    emit('safety', { emergency: state.emergency });
  }

  document.querySelectorAll('[data-go]').forEach((button) => button.addEventListener('click', () => goTo(Number(button.dataset.go) - 1)));
  $('prev-slide').addEventListener('click', () => goTo(state.index - 1));
  $('next-slide').addEventListener('click', () => goTo(state.index + 1));
  $('calibration-demo').addEventListener('click', () => { goToId('lab'); calibrate(); });
  $('lab-calibrate').addEventListener('click', calibrate);
  $('lab-stop').addEventListener('click', emergencyStop);
  $('height-range').addEventListener('input', (event) => { state.z = Number(event.target.value); $('height-readout').textContent = `${state.z.toFixed(1)} cm`; $('pose-z').textContent = `+${(state.z / 100).toFixed(2)}`; emit('command', { type: 'set-height', z: state.z }); });
  $('language-button').addEventListener('click', () => { toast('Pacote EN preparado para a próxima edição da apresentação'); });
  $('fullscreen-button').addEventListener('click', () => { if (!document.fullscreenElement) document.documentElement.requestFullscreen?.(); else document.exitFullscreen?.(); });
  document.addEventListener('keydown', (event) => { if (event.key === 'ArrowRight' || event.key === 'PageDown') { event.preventDefault(); goTo(state.index + 1); } if (event.key === 'ArrowLeft' || event.key === 'PageUp') { event.preventDefault(); goTo(state.index - 1); } if (event.key === ' ') { event.preventDefault(); goTo(1); } if (event.key.toLowerCase() === 'l') goToId('lab'); if (event.key === 'Escape' && state.emergency) emergencyStop(); });
  window.ArmTankAPI = { getState: () => structuredClone(state), command: (type, payload = {}) => { emit('command', { type, ...payload }); log(`Comando externo recebido: ${type}.`, 'API'); }, calibrate, emergencyStop, on: (event, handler) => window.addEventListener(`armtank:${event}`, (customEvent) => handler(customEvent.detail)) };

  function tick() { state.frame += 1; drawPixelRover($('hero-canvas')); drawPixelRover($('lab-canvas'), true); drawKinematics(); drawVideo(); updateTelemetry(); requestAnimationFrame(tick); }
  goToId(location.hash.slice(1) || 'hero');
  bindJoystick();
  tick();
})();
