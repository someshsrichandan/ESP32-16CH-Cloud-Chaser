#pragma once
#include <Arduino.h>

const char PCSS[] PROGMEM = R"===(<style>
:root{--bg:#080812;--cd:#0e0e1e;--br:#1c1c34;--tx:#dde4f0;--dm:#445070;--cyan:#22d3ee;--purple:#a78bfa;}
*{box-sizing:border-box;margin:0;padding:0;}
body{background:var(--bg);color:var(--tx);font-family:'Segoe UI',system-ui,-apple-system,sans-serif;min-height:100vh;}
header{background:linear-gradient(135deg,#140630,#051030);padding:14px;text-align:center;border-bottom:2px solid #241860;}
h1{font-size:1.35rem;background:linear-gradient(90deg,#a78bfa,#22d3ee,#a78bfa);background-size:200%;
   -webkit-background-clip:text;-webkit-text-fill-color:transparent;font-weight:900;animation:sh 3s linear infinite;}
@keyframes sh{0%{background-position:200%}100%{background-position:-200%}}
header p{color:var(--dm);font-size:.74rem;margin-top:4px;}
.w{max-width:960px;margin:0 auto;padding:12px;}
.card{background:var(--cd);border:1px solid var(--br);border-radius:14px;padding:14px;margin-bottom:12px;box-shadow:0 4px 20px rgba(0,0,0,0.5);}
.card h2{font-size:.74rem;color:var(--cyan);margin-bottom:10px;text-transform:uppercase;letter-spacing:.12em;}
.sb{display:flex;flex-wrap:wrap;gap:7px;justify-content:center;align-items:center;}
.bx{padding:5px 12px;border-radius:20px;font-size:.72rem;font-weight:700;border:1px solid;display:inline-flex;align-items:center;}
.be{background:#1a1740;border-color:#4c1d95;color:#c4b5fd;}
.bs{background:#091e14;border-color:#065f46;color:#6ee7b7;}
.bm{background:#15151f;border-color:#374151;color:#9ca3af;}
.bm.au{background:#0a1e0a;border-color:#16a34a;color:#4ade80;}
.bm.mn{background:#1e120a;border-color:#b45309;color:#fcd34d;}
.bm.of{background:#1e0a0a;border-color:#991b1b;color:#f87171;}
.bm.on{background:#062312;border-color:#059669;color:#34d399;box-shadow:0 0 10px rgba(52,211,153,.35);}
.dot{width:8px;height:8px;border-radius:50%;background:#ef4444;display:inline-block;margin-right:6px;}
.dot.ok{background:#22c55e;box-shadow:0 0 8px #22c55e;animation:pu 1.4s ease-in-out infinite;}
@keyframes pu{0%,100%{opacity:1;transform:scale(1)}50%{opacity:.3;transform:scale(.7)}}
.row{display:flex;flex-wrap:wrap;gap:7px;justify-content:center;margin-bottom:9px;}
.btn{padding:9px 14px;border:none;border-radius:9px;cursor:pointer;font-size:.78rem;font-weight:800;
     touch-action:manipulation;-webkit-tap-highlight-color:transparent;user-select:none;
     transition:transform .08s,filter .1s;letter-spacing:.02em;}
.btn:active{transform:scale(.90);filter:brightness(.75);}
.gn{background:linear-gradient(135deg,#16a34a,#14532d);color:#fff;}
.rd{background:linear-gradient(135deg,#dc2626,#7f1d1d);color:#fff;}
.bl{background:linear-gradient(135deg,#2563eb,#1e3a8a);color:#fff;}
.pu{background:linear-gradient(135deg,#7c3aed,#4c1d95);color:#fff;}
.am{background:linear-gradient(135deg,#d97706,#78350f);color:#fff;}
.te{background:linear-gradient(135deg,#0891b2,#164e63);color:#fff;}
.gr{background:linear-gradient(135deg,#4b5563,#1f2937);color:#fff;}
.hi{outline:2px solid rgba(167,139,250,.9);outline-offset:2px;}
.btn.gn.hi{outline:2.5px solid #34d399;outline-offset:2px;box-shadow:0 0 14px rgba(52,211,153,.7);}
.btn.rd.hi{outline:2.5px solid #f87171;outline-offset:2px;box-shadow:0 0 14px rgba(248,113,113,.7);}
.amhi{outline:2px solid rgba(251,191,36,.9);outline-offset:2px;}
.slr{display:flex;flex-wrap:wrap;gap:14px;justify-content:center;margin-top:10px;}
.sw{display:flex;align-items:center;gap:9px;}
.sw label{font-size:.72rem;color:var(--dm);white-space:nowrap;}
input[type=range]{-webkit-appearance:none;height:5px;border-radius:3px;background:#1e293b;outline:none;width:130px;}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:17px;height:17px;border-radius:50%;background:#7c3aed;cursor:pointer;border:2px solid #a78bfa;}
.sv{min-width:38px;text-align:center;font-size:.80rem;color:#a78bfa;font-weight:800;}
.eg{display:grid;grid-template-columns:repeat(auto-fill,minmax(110px,1fr));gap:6px;}
.eb{border:1px solid #2a2860;border-radius:9px;padding:9px 4px;cursor:pointer;font-size:.68rem;font-weight:700;
    background:#12104a;color:#c4b5fd;touch-action:manipulation;-webkit-tap-highlight-color:transparent;
    user-select:none;transition:transform .08s;}
.eb:active{transform:scale(.88);}
.eb.on{background:linear-gradient(135deg,#7c3aed,#4338ca);color:#fff;border-color:#7c3aed;box-shadow:0 0 12px #7c3aed66;}
/* Preset buttons */
.pg{display:grid;grid-template-columns:repeat(auto-fill,minmax(160px,1fr));gap:6px;margin-bottom:10px;}
.pb{border:1px solid #1e293b;border-radius:8px;padding:8px;cursor:pointer;font-size:.72rem;font-weight:700;
    background:#111827;color:#94a3b8;transition:transform .08s;}
.pb:active{transform:scale(.90);}
.pb.on{background:linear-gradient(135deg,#0e7490,#155e75);border-color:#22d3ee;color:#fff;box-shadow:0 0 10px #22d3ee44;}
/* Relay grid */
.rg{display:grid;grid-template-columns:repeat(8,1fr);gap:6px;margin-bottom:6px;}
.rb{border:2px solid #1a2338;border-radius:11px;padding:8px 2px;cursor:pointer;
    touch-action:manipulation;-webkit-tap-highlight-color:transparent;user-select:none;
    min-height:58px;display:flex;flex-direction:column;justify-content:center;align-items:center;gap:3px;
    background:#0c1220;transition:transform .08s,box-shadow .1s;}
.rb:active{transform:scale(.85)!important;}
.rb.off{border-color:#1a2338;color:#2d4060;}
.rb.on{background:linear-gradient(160deg,#166534,#052e16);color:#fff;border-color:#22c55e;box-shadow:0 0 16px #22c55e66;}
.rch{font-size:.80rem;font-weight:900;}
.rst{font-size:.62rem;font-weight:700;letter-spacing:.05em;opacity:.85;}
.rb.off .rst{color:#334155;}
.rb.on .rst{color:#4ade80;}
.notice{background:#101026;border:1px solid #312e81;border-radius:10px;padding:10px 14px;margin-bottom:12px;font-size:.72rem;line-height:1.5;color:#c7d2fe;display:flex;justify-content:space-between;flex-wrap:wrap;gap:8px;}
@media(max-width:540px){
  .rg{grid-template-columns:repeat(4,1fr);}
  .eg{grid-template-columns:repeat(2,1fr);}
  .pg{grid-template-columns:1fr;}
  .rb{min-height:52px;}
}
</style>)===";

const char PHTML[] PROGMEM = R"===(
<header>
  <h1>&#9889; ESP32 &#215; 16-CH RELAY PRO</h1>
  <p>25 Effects &bull; 5 Presets &bull; Non-Blocking v8.5</p>
</header>
<div class="w">

<div class="notice">
  <div><b>AP Hotspot:</b> <span id="ipAP">192.168.4.1</span> (SSID: ESP32_Relay)</div>
  <div><b>Home WiFi:</b> <span id="ipSTA">Connecting...</span></div>
</div>

<div class="card">
  <div class="sb">
    <span class="bx"><span class="dot" id="dot"></span><span id="wst">Connecting...</span></span>
    <span class="bx be" id="sE">Left Chaser</span>
    <span class="bx bs" id="sS">120ms</span>
    <span class="bx bm au" id="sM">AUTO RUN</span>
    <span class="bx be" id="sP">Preset 1</span>
  </div>
</div>

<div class="card">
  <h2>&#127917; Lighting Presets</h2>
  <div class="pg" id="pg"></div>
</div>

<div class="card">
  <h2>&#9881; Playback & Master Controls</h2>
  <div class="row">
    <button class="btn gn" id="bAllOn" onclick="cmd('all_on')">&#9899; All ON</button>
    <button class="btn rd" id="bAllOff" onclick="cmd('all_off')">&#9898; All OFF</button>
    <button class="btn bl" id="bR" onclick="cmd('toggle_run')">&#9646;&#9646; Pause</button>
    <button class="btn pu" id="bA" onclick="cmd('toggle_auto')">&#8635; Auto: ON</button>
    <button class="btn am" onclick="cmd('prev')">&#9664; Prev FX</button>
    <button class="btn te" onclick="cmd('next')">Next FX &#9654;</button>
  </div>
  <div class="row">
    <button class="btn gn" onclick="cmd('odd')">Odd (1,3,5...)</button>
    <button class="btn bl" onclick="cmd('even')">Even (2,4,6...)</button>
    <button class="btn gr" onclick="cmd('invert')">&#8646; Invert Matrix</button>
    <button class="btn am" id="bMn" onclick="cmd('toggle_manual')">Manual Override</button>
  </div>
  <div class="slr">
    <div class="sw">
      <label>&#9203; Speed</label>
      <input type="range" id="slS" min="20" max="500" value="120" oninput="setSpd(this.value)">
      <span class="sv" id="vS">120ms</span>
    </div>
    <div class="sw">
      <label>&#8635; Auto Cycle</label>
      <input type="range" id="slC" min="2" max="30" value="8" oninput="setCyc(this.value)">
      <span class="sv" id="vC">8s</span>
    </div>
  </div>
</div>

<div class="card">
  <h2>&#128268; Direct Relay Override (1-16)</h2>
  <div class="rg" id="rg"></div>
</div>

<div class="card">
  <h2>&#127878; 25 Standalone Effects</h2>
  <div class="eg" id="eg"></div>
</div>
</div>
)===";

const char PJS[] PROGMEM = R"===(
<script>
var EN=["Left Chaser","Right Chaser","Bounce","Center Out","Edges In","Zip+Unzip","Double Dot","Binary Ctr","Sparkle","Twinkle","Wave Fill","Heartbeat","Cylon Eye","Alternating","Cascade","Theater 3P","Rain Drop","Fireworks","Knight Rider","Meteor","Dual Bounce","Cross Fade","Strobe Flash","Wipe Fill","Stack Build"];
var PN=["🌟 Showcase (All 25)","⚡ High Energy Strobes","🌊 Waves & Chasers","🌧️ Fireworks & Rain","🎭 Matrix Alternating"];
var S={e:0,s:120,m:"auto",c:8,p:0,pn:PN[0],ap:"192.168.4.1",sta:"Connecting...",ch:Array(16).fill(0)};
var PT=700,fails=0,spdTimeout=null,cycTimeout=null;
var rPend=Array(16).fill(false);

// Generate Preset buttons
var pg=document.getElementById('pg');
PN.forEach(function(n,i){
  var b=document.createElement('button');
  b.className='pb'+(i===0?' on':'');
  b.id='pb'+i;b.textContent=n;
  b.onclick=function(){req('/preset?id='+i,apply);};
  pg.appendChild(b);
});

// Generate Effect buttons
var eg=document.getElementById('eg');
EN.forEach(function(n,i){
  var b=document.createElement('button');
  b.className='eb'+(i===0?' on':'');
  b.id='eb'+i;b.textContent=(i+1)+'. '+n;
  b.onclick=function(){req('/effect?id='+i,apply);};
  eg.appendChild(b);
});

// Generate Relay buttons
var rg=document.getElementById('rg');
for(var i=0;i<16;i++){(function(x){
  var b=document.createElement('button');
  b.className='rb off';b.id='rb'+x;
  b.innerHTML='<span class="rch">CH'+(x+1)+'</span><span class="rst" id="rs'+x+'">OFF</span>';
  b.onclick=function(){
    if(rPend[x])return;
    rPend[x]=true;
    var nv=S.ch[x]?0:1;
    setRUI(x,nv);
    req('/relay?ch='+x+'&v='+nv,
      function(d){rPend[x]=false;apply(d);},
      function(){rPend[x]=false;setRUI(x,S.ch[x]);}
    );
  };
  rg.appendChild(b);
})(i);}

function setRUI(i,v){
  var el=document.getElementById('rb'+i);
  if(el){el.className='rb '+(v?'on':'off');}
  var st=document.getElementById('rs'+i);
  if(st){st.textContent=v?'ON':'OFF';}
}

function cmd(a){req('/cmd?a='+a,apply);}

function setSpd(v){
  document.getElementById('vS').textContent=v+'ms';
  clearTimeout(spdTimeout);
  spdTimeout=setTimeout(function(){req('/speed?v='+v,apply);},150);
}

function setCyc(v){
  document.getElementById('vC').textContent=v+'s';
  clearTimeout(cycTimeout);
  cycTimeout=setTimeout(function(){req('/cycle?v='+(v*1000),apply);},200);
}

function req(url,ok,err){
  var x=new XMLHttpRequest();
  x.timeout=3000;
  x.open('GET',url,true);
  x.onreadystatechange=function(){
    if(x.readyState!==4)return;
    if(x.status===200){
      try{var d=JSON.parse(x.responseText);fails=0;PT=700;if(ok)ok(d);}
      catch(e){if(err)err();}
    }else{if(err)err();onFail();}
  };
  x.ontimeout=x.onerror=function(){if(err)err();onFail();};
  x.send();
}

function onFail(){fails++;PT=Math.min(3000,700+fails*400);setConn(false);}

function setConn(ok){
  document.getElementById('dot').className='dot'+(ok?' ok':'');
  document.getElementById('wst').textContent=ok?'Connected':'Reconnecting...';
}

function apply(d){
  S=d;setConn(true);
  document.getElementById('ipAP').textContent=S.ap||'192.168.4.1';
  document.getElementById('ipSTA').textContent=(S.sta&&S.sta!=='0.0.0.0')?S.sta:'Not Connected (Using AP)';

  PN.forEach(function(_,i){var el=document.getElementById('pb'+i);if(el)el.className='pb'+(S.p===i?' on':'');});
  EN.forEach(function(_,i){var el=document.getElementById('eb'+i);if(el)el.className='eb'+(S.e===i?' on':'');});

  for(var i=0;i<16;i++){if(!rPend[i])setRUI(i,S.ch[i]);}

  document.getElementById('sE').textContent=EN[S.e]||'Effect';
  document.getElementById('sS').textContent='Speed: '+S.s+'ms';
  document.getElementById('sP').textContent=PN[S.p]||'Preset';

  var isAuto=S.m==='auto';
  var isMan=S.m==='manual';
  var isOff=S.m==='off'||S.m==='stopped';
  var isOn=S.m==='all_on';

  var mt=isOn?'ALL ON':isOff?'ALL OFF':isMan?'MANUAL':isAuto?'AUTO RUN':'SINGLE FX';
  document.getElementById('sM').textContent=mt;
  document.getElementById('sM').className='bx bm '+(isOn?'on':isOff?'of':isMan?'mn':isAuto?'au':'be');

  var bOn=document.getElementById('bAllOn'), bOff=document.getElementById('bAllOff');
  if(bOn) bOn.className='btn gn'+(isOn?' hi':'');
  if(bOff) bOff.className='btn rd'+(isOff?' hi':'');

  document.getElementById('bR').textContent=(isOff||isOn)?'&#9654; Resume':'&#9646;&#9646; Pause';
  document.getElementById('bA').textContent='&#8635; Auto: '+(isAuto?'ON':'OFF');
  document.getElementById('bA').className='btn pu'+(isAuto?' hi':'');

  document.getElementById('bMn').textContent=isMan?'Exit Manual':'Manual Override';
  document.getElementById('bMn').className='btn am'+(isMan?' amhi':'');

  var ss=document.getElementById('slS'),sc=document.getElementById('slC');
  if(document.activeElement!==ss)ss.value=S.s;
  if(document.activeElement!==sc)sc.value=S.c;
}

function poll(){req('/state',apply,null);setTimeout(poll,PT);}
setTimeout(poll,1000);
</script>
)===";

