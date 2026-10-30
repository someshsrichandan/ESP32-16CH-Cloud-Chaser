#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
<title>ESP32 16CH Relay Controller</title>
<style>
:root {
  --bg: #090a15;
  --card: #101223;
  --border: #1c203c;
  --accent: #8b5cf6;
  --accent2: #06b6d4;
  --success: #10b981;
  --danger: #ef4444;
  --warning: #f59e0b;
  --text: #f1f5f9;
  --muted: #64748b;
}
* { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif; }
body { background: var(--bg); color: var(--text); padding-bottom: 30px; -webkit-tap-highlight-color: transparent; }
header { background: linear-gradient(135deg, #1b0938, #071536); padding: 18px 12px; text-align: center; border-bottom: 2px solid #2d1d5e; position: relative; }
h1 { font-size: 1.4rem; font-weight: 900; background: linear-gradient(90deg, #c084fc, #38bdf8, #a855f7); -webkit-background-clip: text; -webkit-text-fill-color: transparent; letter-spacing: 0.05em; }
.sub { font-size: 0.75rem; color: #94a3b8; margin-top: 5px; font-weight: 600; }
.wrap { max-width: 980px; margin: 0 auto; padding: 12px; }
.card { background: var(--card); border: 1px solid var(--border); border-radius: 14px; padding: 14px; margin-bottom: 12px; box-shadow: 0 4px 15px rgba(0,0,0,0.3); }
.card-title { font-size: 0.78rem; text-transform: uppercase; color: var(--accent2); font-weight: 800; letter-spacing: 0.1em; margin-bottom: 12px; display: flex; justify-content: space-between; align-items: center; }

/* Status Bar */
.status-bar { display: flex; flex-wrap: wrap; gap: 8px; justify-content: center; align-items: center; }
.badge { padding: 6px 12px; border-radius: 20px; font-size: 0.73rem; font-weight: 700; border: 1px solid var(--border); background: #13172e; display: inline-flex; align-items: center; gap: 6px; }
.b-purple { background: #201344; border-color: #581c87; color: #d8b4fe; }
.b-green  { background: #062b1b; border-color: #047857; color: #6ee7b7; }
.b-cyan   { background: #082433; border-color: #0e7490; color: #67e8f9; }
.b-amber  { background: #2e1a05; border-color: #b45309; color: #fcd34d; }
.dot { width: 9px; height: 9px; border-radius: 50%; background: var(--danger); }
.dot.ok { background: var(--success); box-shadow: 0 0 8px var(--success); animation: pulse 1.5s infinite; }
@keyframes pulse { 0%,100%{opacity:1;transform:scale(1);} 50%{opacity:0.4;transform:scale(0.8);} }

/* 16-Relay Grid */
.relay-grid { display: grid; grid-template-columns: repeat(8, 1fr); gap: 7px; margin-bottom: 4px; }
.relay-cell { background: #0c0e1c; border: 2px solid #1a2035; border-radius: 10px; padding: 9px 2px; text-align: center; cursor: pointer; transition: transform .08s, box-shadow .15s; user-select: none; }
.relay-cell:active { transform: scale(0.90); }
.relay-cell.on { background: linear-gradient(145deg, #065f46, #022c22); border-color: #10b981; box-shadow: 0 0 16px rgba(16,185,129,0.45); }
.relay-num { font-size: 0.82rem; font-weight: 800; color: #94a3b8; }
.relay-cell.on .relay-num { color: #fff; }
.relay-state { font-size: 0.65rem; font-weight: 800; color: #475569; margin-top: 3px; }
.relay-cell.on .relay-state { color: #34d399; }

/* Control Buttons */
.btn-row { display: flex; flex-wrap: wrap; gap: 8px; justify-content: center; margin-bottom: 10px; }
.btn { padding: 10px 14px; border: none; border-radius: 10px; font-size: 0.78rem; font-weight: 800; cursor: pointer; color: #fff; transition: transform .08s, filter .1s; display: inline-flex; align-items: center; gap: 5px; user-select: none; }
.btn:active { transform: scale(0.92); filter: brightness(0.8); }
.btn-green  { background: linear-gradient(135deg, #10b981, #047857); }
.btn-red    { background: linear-gradient(135deg, #ef4444, #b91c1c); }
.btn-blue   { background: linear-gradient(135deg, #3b82f6, #1d4ed8); }
.btn-purple { background: linear-gradient(135deg, #8b5cf6, #6d28d9); }
.btn-amber  { background: linear-gradient(135deg, #f59e0b, #b45309); }
.btn-cyan   { background: linear-gradient(135deg, #06b6d4, #0e7490); }
.btn-dark   { background: #1f2544; border: 1px solid #333d6b; }
.btn.active { outline: 3px solid rgba(139,92,246,0.8); outline-offset: 2px; }

/* Sliders */
.sliders { display: flex; flex-wrap: wrap; gap: 16px; justify-content: center; margin-top: 10px; padding: 8px; background: #0c0e1c; border-radius: 10px; }
.slider-group { display: flex; align-items: center; gap: 10px; }
.slider-group label { font-size: 0.74rem; color: #94a3b8; font-weight: 700; white-space: nowrap; }
input[type=range] { -webkit-appearance: none; height: 6px; border-radius: 3px; background: #222949; outline: none; width: 140px; }
input[type=range]::-webkit-slider-thumb { -webkit-appearance: none; width: 18px; height: 18px; border-radius: 50%; background: var(--accent); cursor: pointer; border: 2px solid #c4b5fd; box-shadow: 0 0 8px rgba(139,92,246,0.6); }
.slider-val { font-size: 0.8rem; font-weight: 800; color: var(--accent2); min-width: 46px; text-align: center; }

/* Countdown bar */
.cd-bar { width: 100%; height: 5px; background: #1c203c; border-radius: 3px; overflow: hidden; margin-top: 10px; }
.cd-fill { height: 100%; background: linear-gradient(90deg, #38bdf8, #8b5cf6); width: 0%; transition: width 0.3s; }

/* 40 Effect Grid */
.effects-grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(130px, 1fr)); gap: 6px; }
.fx-btn { background: #13172e; border: 1px solid #222a52; border-radius: 8px; padding: 10px 6px; font-size: 0.72rem; font-weight: 700; color: #cbd5e1; cursor: pointer; text-align: center; transition: all .1s; user-select: none; }
.fx-btn:active { transform: scale(0.92); }
.fx-btn.active { background: linear-gradient(135deg, #7c3aed, #4f46e5); color: #fff; border-color: #a78bfa; box-shadow: 0 0 14px rgba(124,58,237,0.5); font-weight: 900; }

/* WiFi & Setup Panel */
.form-row { display: flex; flex-wrap: wrap; gap: 8px; margin-bottom: 10px; align-items: center; }
.form-input { flex: 1; min-width: 150px; background: #0a0c18; border: 1px solid #252c4d; border-radius: 8px; padding: 9px 12px; color: #fff; font-size: 0.82rem; outline: none; }
.form-input:focus { border-color: var(--accent); }
.guide-box { background: #0c1020; border: 1px solid #202747; border-radius: 8px; padding: 12px; font-size: 0.74rem; line-height: 1.6; color: #cbd5e1; margin-top: 10px; }
.guide-box b { color: var(--accent2); }
.guide-box code { background: #1e2648; padding: 2px 6px; border-radius: 4px; color: #fcd34d; font-family: monospace; }

@media(max-width: 600px) {
  .relay-grid { grid-template-columns: repeat(4, 1fr); }
  .effects-grid { grid-template-columns: repeat(2, 1fr); }
  .sliders { flex-direction: column; align-items: stretch; }
  .slider-group { justify-content: space-between; }
}
</style>
</head>
<body>

<header>
  <h1>&#9889; ESP32 &#215; 16-CH RELAY PRO</h1>
  <div class="sub">40 FX &bull; Dual WiFi &bull; Auto Continuous &bull; Cool-Run v9.0</div>
</header>

<div class="wrap">

  <!-- Status Bar -->
  <div class="card">
    <div class="status-bar">
      <span class="badge"><span class="dot" id="dot"></span><span id="conn-text">Connecting...</span></span>
      <span class="badge b-purple" id="dev-tag">&#127380; ID: relay16_...</span>
      <span class="badge" id="wifi-status-tag">&#128246; WiFi: Checking...</span>
      <span class="badge" id="cloud-status-tag">&#9729; Cloud: Offline</span>
      <span class="badge b-purple" id="fx-name">Left Chaser</span>
      <span class="badge b-cyan" id="speed-tag">120ms</span>
      <span class="badge b-green" id="mode-tag">FX Mode</span>
      <span class="badge b-amber" id="temp-tag">--&deg;C</span>
      <span class="badge" id="ip-tag">&#127760; 192.168.4.1</span>
    </div>
    <div class="cd-bar"><div class="cd-fill" id="cd-fill"></div></div>
  </div>

  <!-- Realtime 16 Relays Visualizer / Manual Click -->
  <div class="card">
    <div class="card-title">
      <span>&#128268; 16-Channel Relays (Tap to Toggle)</span>
      <span style="font-size:0.7rem; color:var(--muted)">Optocoupler Safe</span>
    </div>
    <div class="relay-grid" id="relay-grid"></div>
  </div>

  <!-- Master Controls -->
  <div class="card">
    <div class="card-title"><span>&#9881; Controls &amp; Continuous Mode</span></div>
    <div class="btn-row">
      <button class="btn btn-green" onclick="cmd('all_on')">&#9899; All ON</button>
      <button class="btn btn-red"   onclick="cmd('all_off')">&#9898; All OFF</button>
      <button class="btn btn-blue"  id="btn-run" onclick="cmd('toggle_run')">&#9646;&#9646; Pause</button>
      <button class="btn btn-purple" id="btn-auto" onclick="cmd('toggle_auto')">&#8635; Auto: OFF</button>
      <button class="btn btn-cyan"  id="btn-shuf" onclick="cmd('toggle_shuffle')">&#128256; Shuffle: OFF</button>
      <button class="btn btn-amber" onclick="cmd('prev')">&#9664; Prev</button>
      <button class="btn btn-amber" onclick="cmd('next')">Next &#9654;</button>
    </div>
    <div class="btn-row">
      <button class="btn btn-dark" onclick="cmd('odd')">Odd (1,3..)</button>
      <button class="btn btn-dark" onclick="cmd('even')">Even (2,4..)</button>
      <button class="btn btn-dark" onclick="cmd('invert')">&#8646; Invert</button>
      <button class="btn btn-amber" id="btn-man" onclick="cmd('toggle_manual')">Manual Mode</button>
    </div>

    <!-- Sliders -->
    <div class="sliders">
      <div class="slider-group">
        <label>&#9889; FX Speed</label>
        <input type="range" id="sl-spd" min="40" max="600" value="120" oninput="onSpeedChange(this.value)">
        <span class="slider-val" id="val-spd">120ms</span>
      </div>
      <div class="slider-group">
        <label>&#8635; Auto Cycle Duration</label>
        <input type="range" id="sl-cyc" min="3" max="60" value="8" oninput="onCycleChange(this.value)">
        <span class="slider-val" id="val-cyc">8s</span>
      </div>
    </div>
  </div>

  <!-- 40 Effects Selector -->
  <div class="card">
    <div class="card-title">
      <span>&#127878; 40 Light &amp; Chaser Effects</span>
      <span style="font-size:0.7rem; color:var(--muted)">Select any effect</span>
    </div>
    <div class="effects-grid" id="effects-grid"></div>
  </div>

  <!-- WiFi & Internet Settings -->
  <div class="card">
    <div class="card-title">
      <span>&#128246; WiFi Router &amp; Internet Setup</span>
      <button class="btn btn-dark" style="padding:4px 8px; font-size:0.7rem;" onclick="toggleGuide()">&#128214; Internet Setup Guide</button>
    </div>

    <!-- Live Diagnostics & Device ID Card -->
    <div style="background:#090d1f; border:1px solid #1f274a; border-radius:10px; padding:12px; margin-bottom:12px;">
      <div style="display:flex; justify-content:space-between; align-items:center; flex-wrap:wrap; gap:8px;">
        <div>
          <span style="font-size:0.7rem; color:#94a3b8; font-weight:700; text-transform:uppercase;">Hardware Device ID (Zero Port Forwarding):</span>
          <div style="font-size:1.1rem; font-weight:900; font-family:monospace; color:#38bdf8; letter-spacing:0.05em;" id="disp-dev-id">relay16_...</div>
        </div>
        <button class="btn btn-purple" style="padding:6px 12px; font-size:0.75rem;" onclick="copyDevId()">&#128203; Copy Device ID</button>
      </div>
      <div style="margin-top:10px; padding-top:10px; border-top:1px solid #171d38; display:grid; grid-template-columns:repeat(auto-fit, minmax(130px, 1fr)); gap:10px; font-size:0.74rem;">
        <div><span style="color:#94a3b8;">Router WiFi:</span><br><b id="disp-wifi-sta" style="color:#fcd34d;">Scanning...</b></div>
        <div><span style="color:#94a3b8;">Local Station IP:</span><br><b id="disp-wifi-ip" style="color:#6ee7b7; font-family:monospace;">Not Connected</b></div>
        <div><span style="color:#94a3b8;">Cloud MQTT:</span><br><b id="disp-cloud-st" style="color:#f87171;">Offline</b></div>
        <div><span style="color:#94a3b8;">Web Panel Link:</span><br><b style="color:#c084fc;">Next.js Cloud Ready</b></div>
      </div>
    </div>

    <p style="font-size:0.75rem; color:#94a3b8; margin-bottom:10px;">
      Save your home WiFi details below. The ESP32 will <b>permanently save</b> them in NVS flash and <b>automatically auto-connect &amp; reconnect</b> every time it powers on!
    </p>

    <div class="form-row">
      <select id="wifi-scan-list" class="form-input" style="max-width:200px;" onchange="document.getElementById('wifi-ssid').value=this.value;">
        <option value="">-- Available Networks --</option>
      </select>
      <button class="btn btn-cyan" onclick="scanWiFi()">&#128269; Scan</button>
    </div>

    <div class="form-row">
      <input type="text" id="wifi-ssid" class="form-input" placeholder="Home WiFi SSID (Network Name)">
      <input type="password" id="wifi-pass" class="form-input" placeholder="WiFi Password">
      <button class="btn btn-green" onclick="saveWiFi()">&#128190; Save &amp; Connect</button>
      <button class="btn btn-red" onclick="resetWiFi()">Clear</button>
    </div>

    <div id="guide-section" class="guide-box" style="display:none;">
      <b>&#127760; HOW TO ACCESS OVER THE INTERNET (FROM ANYWHERE):</b><br><br>
      <b>Method 1: Cloudflare Tunnel / Ngrok (Recommended - 100% Free &amp; Secure)</b><br>
      1. Run free Ngrok or Cloudflare on any home PC or Raspberry Pi on the same WiFi:<br>
      &nbsp;&nbsp;&nbsp;&nbsp;<code>ngrok http <span id="guide-ip">192.168.1.xxx</span>:80</code><br>
      2. Ngrok gives you a public HTTPS URL like <code>https://your-name.ngrok-free.app</code>.<br>
      3. Open that link from any phone or computer in the world on 4G/5G!<br><br>

      <b>Method 2: Router Port Forwarding + DuckDNS</b><br>
      1. Set a DHCP static IP for ESP32 in your router (e.g. <code>192.168.1.150</code>).<br>
      2. In your home router settings, forward External Port <code>8080</code> to Internal IP <code>192.168.1.150</code> Port <code>80</code>.<br>
      3. Use free dynamic DNS from <a href="https://www.duckdns.org" target="_blank" style="color:#38bdf8">DuckDNS.org</a> (e.g. <code>myrelay.duckdns.org</code>).<br>
      4. Access anywhere: <code>http://myrelay.duckdns.org:8080</code>.<br><br>

      <b>Method 3: REST API for Siri, Alexa, Google Home &amp; Home Assistant</b><br>
      &bull; <code>GET /api/effect?id=5</code> (trigger any effect)<br>
      &bull; <code>GET /api/cmd?a=all_on</code> | <code>all_off</code> | <code>next</code><br>
      &bull; <code>GET /api/relay?ch=0&amp;v=1</code> (toggle specific channel)
    </div>
  </div>

</div>

<script>
  var FX_NAMES = [
    "Left Chaser","Right Chaser","Ping Pong","Center Out","Edges In","Zip + Unzip",
    "Dual Snake","Binary Counter","Sparkle / Stardust","Twinkle Sky","Wave Fill",
    "Heartbeat Pulse","Cylon Scanner","Alternating Odd/Ev","Falling Dominos","Theater Marquee",
    "Raindrop Ripple","Fireworks Burst","Knight Rider","Meteor Shower","Double Bounce",
    "Cross Over","Strobe Burst","Stack Build","Reverse Stack","Quad Runner",
    "In-Out Breathe","Segment Stride","Police Strobe","Curtain Reveal","Cyber Glitch",
    "Ripple Reflect","Marching Pairs","Converge & Cross","4-Phase Wave","Harbor Radar",
    "Fill & Drain","Random Walk","DNA Helix","All Breather"
  ];

  var state = { e:0, s:120, a:0, sh:0, r:1, m:0, c:8, cd:0, temp:0, ch:Array(16).fill(0), wifi:{} };
  var pendingToggles = Array(16).fill(false);
  var pollInterval = 650;
  var fails = 0;

  // Render 16 Relay cells
  var rGrid = document.getElementById('relay-grid');
  for (var i = 0; i < 16; i++) {
    (function(ch) {
      var d = document.createElement('div');
      d.className = 'relay-cell';
      d.id = 'rc-' + ch;
      d.innerHTML = '<div class="relay-num">CH' + (ch + 1) + '</div><div class="relay-state" id="rst-' + ch + '">OFF</div>';
      d.onclick = function() {
        if (pendingToggles[ch]) return;
        pendingToggles[ch] = true;
        var newVal = state.ch[ch] ? 0 : 1;
        updateRelayCell(ch, newVal);
        fetch('/api/relay?ch=' + ch + '&v=' + newVal)
          .then(function(r){ return r.json(); })
          .then(function(d){ pendingToggles[ch] = false; renderState(d); })
          .catch(function(){ pendingToggles[ch] = false; updateRelayCell(ch, state.ch[ch]); });
      };
      rGrid.appendChild(d);
    })(i);
  }

  // Render 40 FX buttons
  var fxGrid = document.getElementById('effects-grid');
  FX_NAMES.forEach(function(name, idx) {
    var b = document.createElement('button');
    b.className = 'fx-btn' + (idx === 0 ? ' active' : '');
    b.id = 'fxb-' + idx;
    b.textContent = (idx + 1) + '. ' + name;
    b.onclick = function() {
      fetch('/api/effect?id=' + idx)
        .then(function(r){ return r.json(); })
        .then(renderState);
    };
    fxGrid.appendChild(b);
  });

  var updateRelayCell = function(ch, val) {
    var el = document.getElementById('rc-' + ch);
    var st = document.getElementById('rst-' + ch);
    if (val) {
      el.className = 'relay-cell on';
      st.textContent = 'ON';
    } else {
      el.className = 'relay-cell';
      st.textContent = 'OFF';
    }
  };

  var cmd = function(act) {
    fetch('/api/cmd?a=' + act)
      .then(function(r){ return r.json(); })
      .then(renderState);
  };

  var onSpeedChange = function(val) {
    document.getElementById('val-spd').textContent = val + 'ms';
    fetch('/api/speed?v=' + val).then(function(r){ return r.json(); }).then(renderState);
  };

  var onCycleChange = function(val) {
    document.getElementById('val-cyc').textContent = val + 's';
    fetch('/api/cycle?v=' + (val * 1000));
  };

  var toggleGuide = function() {
    var el = document.getElementById('guide-section');
    el.style.display = (el.style.display === 'none') ? 'block' : 'none';
  };

  var scanWiFi = function() {
    var sel = document.getElementById('wifi-scan-list');
    sel.innerHTML = '<option value="">Scanning...</option>';
    fetch('/api/wifi/scan')
      .then(function(r){ return r.json(); })
      .then(function(list) {
        sel.innerHTML = '<option value="">-- Choose Network --</option>';
        list.forEach(function(net) {
          var opt = document.createElement('option');
          opt.value = net.ssid;
          opt.textContent = net.ssid + ' (' + net.rssi + ' dBm)';
          sel.appendChild(opt);
        });
      })
      .catch(function() {
        sel.innerHTML = '<option value="">Scan failed (retry)</option>';
      });
  };

  var saveWiFi = function() {
    var s = encodeURIComponent(document.getElementById('wifi-ssid').value.trim());
    var p = encodeURIComponent(document.getElementById('wifi-pass').value.trim());
    if (!s) { alert('Please enter or select a WiFi SSID'); return; }
    fetch('/api/wifi/save?ssid=' + s + '&pass=' + p)
      .then(function(r){ return r.json(); })
      .then(function(d) {
        alert('WiFi settings saved! ESP32 is connecting to ' + decodeURIComponent(s) + '...');
        renderState(d);
      });
  };

  var resetWiFi = function() {
    if (!confirm('Clear stored WiFi credentials?')) return;
    fetch('/api/wifi/reset')
      .then(function(r){ return r.json(); })
      .then(function(d) {
        alert('WiFi credentials cleared!');
        renderState(d);
      });
  };

  var copyDevId = function() {
    var idText = (state && state.device_id) ? state.device_id : document.getElementById('disp-dev-id').textContent;
    if (navigator.clipboard && navigator.clipboard.writeText) {
      navigator.clipboard.writeText(idText).then(function() {
        alert('Device ID "' + idText + '" copied to clipboard!\nPaste it into your Next.js Web Panel Settings to connect worldwide.');
      });
    } else {
      prompt('Copy your Device ID:', idText);
    }
  };

  var renderState = function(d) {
    state = d;
    fails = 0;
    pollInterval = 650;
    
    // Connection badge
    document.getElementById('dot').className = 'dot ok';
    document.getElementById('conn-text').textContent = 'Connected';

    // Device ID & Copy Card
    if (d.device_id) {
      var dt = document.getElementById('dev-tag');
      if (dt) dt.innerHTML = '&#127380; ' + d.device_id;
      var ddi = document.getElementById('disp-dev-id');
      if (ddi) ddi.textContent = d.device_id;
    }

    // WiFi Station Status
    var wst = document.getElementById('wifi-status-tag');
    var dws = document.getElementById('disp-wifi-sta');
    var dwi = document.getElementById('disp-wifi-ip');
    if (d.wifi && d.wifi.sta_ok) {
      if (wst) { wst.className = 'badge b-green'; wst.innerHTML = '&#128246; ' + d.wifi.sta_ssid; }
      if (dws) { dws.style.color = '#6ee7b7'; dws.textContent = d.wifi.sta_ssid + ' (' + d.wifi.rssi + ' dBm)'; }
      if (dwi) { dwi.textContent = d.wifi.sta_ip; }
    } else if (d.wifi && d.wifi.sta_connecting) {
      if (wst) { wst.className = 'badge b-amber'; wst.innerHTML = '&#128246; Connecting...'; }
      if (dws) { dws.style.color = '#fcd34d'; dws.textContent = 'Connecting to ' + (d.wifi.sta_ssid || 'WiFi') + '...'; }
      if (dwi) { dwi.textContent = 'Acquiring IP...'; }
    } else {
      if (wst) { wst.className = 'badge'; wst.innerHTML = '&#128246; Hotspot Only'; }
      if (dws) { dws.style.color = '#94a3b8'; dws.textContent = 'Not connected (AP Only)'; }
      if (dwi) { dwi.textContent = d.wifi ? d.wifi.ap_ip : '192.168.4.1'; }
    }

    // Cloud MQTT Status
    var cst = document.getElementById('cloud-status-tag');
    var dcs = document.getElementById('disp-cloud-st');
    if (d.mqtt && d.mqtt.connected) {
      if (cst) { cst.className = 'badge b-cyan'; cst.innerHTML = '&#9729; Cloud: Online'; }
      if (dcs) { dcs.style.color = '#67e8f9'; dcs.textContent = 'Connected (broker.emqx.io)'; }
    } else {
      if (cst) { cst.className = 'badge'; cst.innerHTML = '&#9729; Cloud: Offline'; }
      if (dcs) { dcs.style.color = '#f87171'; dcs.textContent = (d.wifi && d.wifi.sta_ok) ? 'Connecting to broker...' : 'Offline (Needs Home WiFi)'; }
    }

    // Effect highlight
    FX_NAMES.forEach(function(_, idx) {
      var btn = document.getElementById('fxb-' + idx);
      if (btn) btn.className = 'fx-btn' + (state.e === idx ? ' active' : '');
    });

    // Relay states
    for (var i = 0; i < 16; i++) {
      if (!pendingToggles[i]) {
        updateRelayCell(i, state.ch[i]);
      }
    }

    // Tags
    document.getElementById('fx-name').textContent = FX_NAMES[state.e] || ('FX ' + state.e);
    document.getElementById('speed-tag').textContent = state.s + 'ms';
    
    var modeStr = state.m ? 'MANUAL' : (state.a ? (state.sh ? 'AUTO SHUFFLE' : 'AUTO SEQ') : 'EFFECT');
    document.getElementById('mode-tag').textContent = modeStr;
    document.getElementById('temp-tag').innerHTML = (state.temp > 0 ? (state.temp + '&deg;C') : 'OK');

    var ipStr = state.wifi.sta_ok ? (state.wifi.sta_ip) : state.wifi.ap_ip;
    document.getElementById('ip-tag').textContent = ipStr;
    document.getElementById('guide-ip').textContent = state.wifi.sta_ok ? state.wifi.sta_ip : '192.168.1.xxx';

    // Master Buttons
    var bRun = document.getElementById('btn-run');
    bRun.innerHTML = state.r ? '&#9646;&#9646; Pause' : '&#9654; Resume';
    
    var bAuto = document.getElementById('btn-auto');
    bAuto.textContent = 'Auto: ' + (state.a ? 'ON' : 'OFF');
    bAuto.className = 'btn btn-purple' + (state.a ? ' active' : '');

    var bShuf = document.getElementById('btn-shuf');
    bShuf.textContent = 'Shuffle: ' + (state.sh ? 'ON' : 'OFF');
    bShuf.className = 'btn btn-cyan' + (state.sh ? ' active' : '');

    var bMan = document.getElementById('btn-man');
    bMan.textContent = state.m ? 'Exit Manual' : 'Manual Mode';
    bMan.className = 'btn btn-amber' + (state.m ? ' active' : '');

    // Sliders sync
    var slSpd = document.getElementById('sl-spd');
    var slCyc = document.getElementById('sl-cyc');
    if (document.activeElement !== slSpd) slSpd.value = state.s;
    if (document.activeElement !== slCyc) slCyc.value = state.c;

    // Countdown Fill
    if (state.a && state.c > 0 && state.cd >= 0) {
      var pct = Math.max(0, Math.min(100, ((state.c - state.cd) / state.c) * 100));
      document.getElementById('cd-fill').style.width = pct + '%';
    } else {
      document.getElementById('cd-fill').style.width = '0%';
    }
  };

  var poll = function() {
    fetch('/api/state')
      .then(function(r) { return r.json(); })
      .then(function(d) {
        renderState(d);
        setTimeout(poll, pollInterval);
      })
      .catch(function() {
        fails++;
        pollInterval = Math.min(3500, 650 + fails * 500);
        document.getElementById('dot').className = 'dot';
        document.getElementById('conn-text').textContent = 'Reconnecting...';
        setTimeout(poll, pollInterval);
      });
  };

  setTimeout(poll, 1200);
</script>
</body>
</html>
)rawliteral";

#endif

