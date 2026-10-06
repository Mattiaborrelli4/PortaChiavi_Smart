#include <pk_dashboard.h>

namespace dashboard {

static const char PAGE[] PROGMEM = R"rawlit(
<!DOCTYPE html>
<html lang="it">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<meta name="theme-color" content="#0b1020">
<title>PortaChiave</title>
<style>
*{margin:0;padding:0;box-sizing:border-box;-webkit-tap-highlight-color:transparent}
body{background:#0b1020;color:#fff;font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,sans-serif;font-size:15px;line-height:1.45}
.wrap{max-width:460px;margin:0 auto;padding-bottom:28px}
.hidden{display:none!important}
.scrbar{position:sticky;top:0;z-index:5;display:flex;align-items:center;gap:8px;padding:10px 12px;background:#0b1020;border-bottom:1px solid #1d2a4a}
.back{min-height:44px;padding:0 14px;background:transparent;border:1px solid #1d2a4a;border-radius:10px;color:#9db2e0;font-size:13px;white-space:nowrap}
.title{flex:1;text-align:center;font-weight:700;letter-spacing:.04em;font-size:15px;text-transform:uppercase}
.hd{text-align:center;padding:22px 16px 14px}
.hd h1{font-size:24px;letter-spacing:.14em;font-weight:800}
.hd .st{margin-top:10px}
.pill{display:inline-flex;align-items:center;gap:6px;padding:5px 12px;border-radius:999px;background:#101a33;color:#cfd9f2;font-size:13px;font-weight:600}
.ddot{width:9px;height:9px;border-radius:50%;background:#6b7ca3;flex:none}
.ddot.on{background:#22c55e;box-shadow:0 0 8px rgba(34,197,94,.7)}
.ddot.off{background:#ef4444;box-shadow:0 0 8px rgba(239,68,68,.7)}
.card{background:#15203c;border:1px solid #1d2a4a;border-radius:14px;margin:12px;padding:12px;overflow:hidden}
.row{display:flex;justify-content:space-between;align-items:center;gap:10px;padding:9px 2px;border-bottom:1px solid #1a2747}
.row:last-child{border-bottom:0}
.row b{font-weight:600;white-space:nowrap}
.row span{color:#8ea0c9;text-align:right;word-break:break-all}
.cap{font-size:11px;text-transform:uppercase;letter-spacing:.12em;color:#5f739f;margin:12px 2px 4px}
button{font:inherit;border:0;border-radius:12px;cursor:pointer;touch-action:manipulation;color:#fff}
button:disabled{opacity:.5}
.big{display:block;width:calc(100% - 24px);min-height:56px;padding:14px 16px;background:#3b82f6;font-size:16px;font-weight:700;text-align:left;border-radius:14px;margin:10px 12px}
.big:active{background:#2f6fe0}
.big .arr{float:right;color:#c6d9ff}
.btn{min-height:56px;padding:12px 18px;background:#3b82f6;font-weight:700;font-size:15px;border-radius:12px}
.btn.alt{background:#232f52;color:#cdd9f5}
.btn.warn{background:#b45309}
.btn.danger{background:#b91c1c}
.alr{background:#3b0a0a;border:1px solid #7f1d1d;color:#fca5a5;border-radius:12px;padding:12px;margin:12px;font-weight:700;text-align:center}
.egrid{display:grid;grid-template-columns:repeat(4,1fr);gap:8px}
.egrid button{min-height:54px;font-size:12px;padding:6px 2px;background:#1a2747;color:#cdd9f5;border-radius:10px}
.egrid button.sel{background:#3b82f6;color:#fff}
.net{background:#111b36;border:1px solid #1d2a4a;border-radius:12px;margin:10px 12px;padding:12px}
.net .n{font-weight:700;font-size:16px;margin-bottom:6px;word-break:break-all}
.net .m{font-size:12px;color:#7d8fc0;word-break:break-all;margin-bottom:8px}
.risk{display:inline-block;padding:3px 10px;border-radius:999px;font-size:12px;font-weight:700;background:#1f2937;color:#9ca3af}
.risk.good{background:#14532d;color:#4ade80}
.risk.norm{background:#422006;color:#facc15}
.risk.high{background:#431407;color:#fb923c}
.risk.crit{background:#450a0a;color:#f87171}
input{font:inherit;width:100%;padding:12px;border-radius:10px;border:1px solid #1d2a4a;background:#0b1020;color:#fff;margin:4px 0 12px}
label{font-size:12px;color:#8ea0c9}
table{width:100%;border-collapse:collapse;font-size:12px;color:#cdd9f5}
td,th{padding:8px 6px;border-bottom:1px solid #1a2747;text-align:left;white-space:nowrap}
th{color:#5f739f;font-size:10px;text-transform:uppercase;letter-spacing:.08em}
.tabwrap{overflow-x:auto}
.note{color:#6b7ca3;font-size:12px;margin:6px 2px;word-break:break-word}
.msg{min-height:20px;font-size:13px;color:#9db2e0;margin:8px 2px;word-break:break-word}
.errbox{text-align:center;padding:38px 20px}
.errbox .ic{font-size:48px}
.errbox h2{margin:14px 0 8px;font-size:18px}
.errline{color:#8ea0c9;font-size:14px;margin:8px 0 18px;word-break:break-all}
.detail{color:#f87171;font-size:12px;line-height:1.5;background:#111b36;border-radius:8px;padding:8px;margin:0 0 16px;text-align:left;word-break:break-all}
.camstate{font-family:monospace;letter-spacing:.12em;text-align:center;color:#22c55e;margin:12px 0}
</style>
</head>
<body>
<div class="wrap">

<div class="screen" id="scr-home">
  <div class="hd">
    <div class="st"><span class="pill"><span class="ddot off" id="homeDot"></span><span id="homeDotTxt">OFFLINE</span></span></div>
    <h1>PORTACHIAVE</h1>
  </div>
  <div class="card" id="homeCard"></div>
  <div class="alr hidden" id="homeAlert">⚠ SECURITY ALERT</div>
  <div>
    <button class="big" data-g="info">📟 INFO DEVICE<span class="arr">›</span></button>
    <button class="big" data-g="auth">🔐 AUTHENTICATION<span class="arr">›</span></button>
    <button class="big" data-g="funcs">⚙️ FUNCTIONS<span class="arr">›</span></button>
    <button class="big hidden" id="mSec" data-g="sec">🛡 SECURITY<span class="arr">›</span></button>
    <button class="big hidden" id="mSet" data-g="set">🔧 SETTINGS<span class="arr">›</span></button>
  </div>
</div>

<div class="screen hidden" id="scr-info">
  <div class="scrbar"><button class="back" data-g="home">← BACK</button><div class="title">INFO DEVICE</div></div>
  <div class="card" id="infoBody"></div>
</div>

<div class="screen hidden" id="scr-auth">
  <div class="scrbar"><button class="back" data-g="home">← BACK</button><div class="title">AUTHENTICATION</div></div>
  <div class="card" id="authBody"></div>
</div>

<div class="screen hidden" id="scr-funcs">
  <div class="scrbar"><button class="back" data-g="home">← BACK</button><div class="title">FUNCTIONS</div></div>
  <div class="body" id="funcsBody"></div>
</div>

<div class="screen hidden" id="scr-robot">
  <div class="scrbar"><button class="back" data-g="home">← BACK</button><div class="title">ROBOTEYES</div></div>
  <div class="card">
    <div class="note" id="robotLine"></div>
    <div class="cap">Emozioni</div>
    <div class="egrid" id="robotEmo"></div>
    <div class="cap">Animazioni</div>
    <div class="egrid" id="robotAnim"></div>
    <div class="cap">Stato</div>
    <div class="egrid" id="robotTog"></div>
    <div class="msg" id="robotMsg"></div>
  </div>
</div>
<div class="screen hidden" id="scr-wifi">
  <div class="scrbar"><button class="back" data-g="home">← BACK</button><div class="title">WI-FI SCANNER</div></div>
  <div>
    <button class="big" id="scanBtn" data-act="scan">📡 SCAN WIFI</button>
  </div>
  <div class="note" style="margin:0 14px">Scansiona le reti Wi-Fi vicine.</div>
</div>

<div class="screen hidden" id="scr-camera">
  <div class="scrbar"><button class="back" data-g="funcs">← BACK</button><div class="title">CAMERA PAIRING</div></div>
  <div class="card">
    <div class="cap">Temporary link</div>
    <div class="camstate" id="camState">IDLE</div>
    <div class="row"><b>Session</b><span id="camSession">-</span></div>
    <div class="row"><b>Expires</b><span id="camExpiry">-</span></div>
    <div class="msg" id="camMsg"></div>
    <button class="btn" style="width:100%" id="camStart" data-act="camerastart">START SESSION</button>
    <button class="btn danger hidden" style="width:100%;margin-top:10px" id="camStop" data-act="camerastop">STOP SESSION</button>
  </div>
</div>

<div class="screen hidden" id="scr-wres">
  <div class="scrbar"><button class="back" data-g="home">← BACK</button><div class="title">WIFI SCANNER</div></div>
  <div class="body" id="wresBody"></div>
</div>

<div class="screen hidden" id="scr-net">
  <div class="scrbar"><button class="back" data-g="funcs">← BACK</button><div class="title">NETWORK PROBE</div></div>
  <div>
    <button class="big" id="netScanBtn" data-act="netscan">📡 SCAN PROFONDO</button>
  </div>
  <div class="note" style="margin:0 14px" id="netNote">Elenco reti + client associati (scan passivo).</div>
  <div class="card" id="netList"></div>
  <div class="card">
    <div class="cap">Wardriving (WiGLE CSV)</div>
    <div class="row"><b>Registrazione</b><span id="netWD">—</span></div>
    <button class="btn" style="width:100%" data-act="wdon">🗺 AVVIA LOG WARDDRIVING</button>
    <button class="btn" style="width:100%" data-act="wdoff">⏹ STOP LOG</button>
    <button class="btn" style="width:100%" data-act="wddl">⬇ SCARICA wardrive.csv</button>
    <button class="btn" style="width:100%" data-act="wdclear">🗑 SVUOTA LOG</button>
    <div class="note">A ogni scan completato le reti nuove vengono aggiunte al CSV (formato WiGLE per wigle.net).</div>
  </div>
</div>

<div class="screen hidden" id="scr-att">
  <div class="scrbar"><button class="back" data-g="funcs">← BACK</button><div class="title">ATTACK</div></div>
  <div class="card">
    <div class="row"><b>Stato</b><span id="attMode">idle</span></div>
    <div class="row"><b>Target</b><span id="attTarget">—</span></div>
    <div class="row"><b>Inviati</b><span id="attSent">0</span></div>
    <div class="row"><b>pkts/s</b><span id="attPps">0</span></div>
    <div class="row"><b>Timeout</b><span id="attRem">—</span></div>
  </div>
  <div class="card">
    <div class="cap">Obiettivo</div>
    <label>BSSID</label><input id="attBssid" placeholder="AA:BB:CC:DD:EE:FF" autocomplete="off">
    <label>Canale</label><input id="attCh" placeholder="vuoto = automatico" autocomplete="off">
    <button class="btn" style="width:100%" data-act="attackone">⚡ DEAUTH TARGET</button>
  </div>
  <div>
    <button class="big warn" data-act="attackall">🌐 DEAUTH TUTTE LE RETI</button>
  </div>
  <div class="card">
    <div class="cap">Beacon spam</div>
    <label>SSID fake</label><input id="attSsid" placeholder="vuoto = SSID dallo scan" autocomplete="off">
    <button class="btn" style="width:100%" data-act="attackbeacon">📶 BEACON SPAM</button>
  </div>
  <div class="card">
    <div class="cap">Flood / random</div>
    <label>SSID (per probe flood)</label><input id="attSsid2" placeholder="vuoto = SSID dello scan" autocomplete="off">
    <button class="btn" style="width:100%" data-act="attackprobe">🌀 PROBE FLOOD</button>
    <button class="btn" style="width:100%" data-act="attackrnd">🎲 BEACON RANDOM</button>
  </div>
  <div style="margin:12px">
    <button class="btn danger" style="width:100%" data-act="attackstop">🛑 STOP ATTACK</button>
  </div>
  <div class="note" style="margin:0 14px">Durante l'attack la radio salta di canale: il keychain può perdere brevemente la connessione e si riallaccia da solo. Testa solo su reti tue.</div>
</div>

<div class="screen hidden" id="scr-evil">
  <div class="scrbar"><button class="back" data-g="funcs">← BACK</button><div class="title">EVIL TWIN</div></div>
  <div class="card" id="evilStatus"></div>
  <div class="card">
    <div class="cap">Nuovo clone</div>
    <label>SSID da clonare</label><input id="evSsid" placeholder="es. Netgear-5G" autocomplete="off">
    <label>Canale</label><input id="evCh" placeholder="vuoto = 1" autocomplete="off">
    <button class="btn" style="width:100%" data-act="evilgo">🎭 AVVIA EVIL TWIN</button>
  </div>
  <div style="margin:12px">
    <button class="btn danger" style="width:100%" data-act="evilstop">🛑 STOP EVIL TWIN</button>
  </div>
  <div class="note" style="margin:0 14px">Clone l'AP senza password e serve una pagina "verifica password": le credenziali inserite vengono salvate in /evil_log.csv e la vittima continua a vedere "password non valida". Solo su reti tue o autorizzate. Durante l'attacco la dashboard resta raggiungibile ma mostra la pagina phish.</div>
</div>

<div class="screen hidden" id="scr-mon">
  <div class="scrbar"><button class="back" data-g="funcs">← BACK</button><div class="title">PACKET MONITOR</div></div>
  <div>
    <button class="big" id="monBtn" data-act="monon">🐾 AVVIA MONITOR</button>
  </div>
  <div class="card" id="monStats"></div>
  <div class="card">
    <div class="cap">Client rilevati</div>
    <div id="monSta"></div>
  </div>
  <div class="card">
    <div class="cap">Probe request (dispositivi vicini in ascolto)</div>
    <div id="monProbe"></div>
  </div>
</div>

<div class="screen hidden" id="scr-sys">
  <div class="scrbar"><button class="back" data-g="home">← BACK</button><div class="title">SYSTEM</div></div>
  <div class="card" id="sysBody"></div>
</div>

<div class="screen hidden" id="scr-ctl">
  <div class="scrbar"><button class="back" data-g="home">← BACK</button><div class="title">CONTROLS</div></div>
  <div class="card" id="ctlList"></div>
  <div class="card">
    <div class="cap">Nuovo controllo</div>
    <label>Nome</label><input id="cName" placeholder="es. led" autocomplete="off">
    <label>Valore</label><input id="cVal" placeholder="es. 1" autocomplete="off">
    <button class="btn" style="width:100%" data-act="send">▶ INVIA</button>
    <div class="msg" id="ctlMsg"></div>
  </div>
</div>

<div class="screen hidden" id="scr-sec">
  <div class="scrbar"><button class="back" data-g="home">← BACK</button><div class="title">SECURITY</div></div>
  <div><button class="big" id="secBtn" data-act="refresh">🔄 AGGIORNA</button></div>
  <div class="card" id="secAlerts"></div>
  <div class="card">
    <div class="cap">Audit Log</div>
    <div class="tabwrap" id="secAudit"></div>
  </div>
  <div class="card">
    <div class="cap">Dispositivi</div>
    <div id="secDev"></div>
  </div>
</div>

<div class="screen hidden" id="scr-set">
  <div class="scrbar"><button class="back" data-g="home">← BACK</button><div class="title">SETTINGS</div></div>
  <div class="card" id="setBody"></div>
</div>

<div class="screen hidden" id="scr-err">
  <div class="scrbar"><button class="back" data-g="home">← BACK</button><div class="title">ERROR</div></div>
  <div class="errbox">
    <div class="ic">⚠</div>
    <h2>Qualcosa è andato storto</h2>
    <div class="errline" id="errCode"></div>
    <div class="detail hidden" id="errDetail"></div>
    <button class="btn" style="width:100%" data-act="retry">🔁 RIPROVA</button>
  </div>
</div>

</div>
<script>
var S={screen:'home',role:'guest',auth:0,left:0,cid:'—',did:'—',mods:0,alerts:0,nets:[],decoy:false,em:'neutral',sleep:false,breath:false,fps:0,retry:'home',upBase:0,upAt:0,HT:0,PH:null,nets2:[],stas:[],mode2:'',camera:null};
function $(id){return document.getElementById(id);}
function esc(s){return String(s==null?'':s).replace(/[&<>"']/g,function(c){return {'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c];});}
function jget(r){return (r&&typeof r==='object')?r:null;}
function fmtUpt(t){t=Math.max(0,t|0);var h=(t/3600|0),m=(t%3600)/60|0,s=t%60;function p(n){return (n<10?'0':'')+n;}return p(h)+':'+p(m)+':'+p(s);}
function api(url,opts){
  opts=opts||{};
  return new Promise(function(res){
    var p={method:opts.method||'GET',headers:{'Accept':'application/json'}};
    if(opts.body){p.method='POST';p.headers['Content-Type']='application/json';p.body=opts.body;}
    fetch(url,p).then(function(r){
      r.text().then(function(t){
        var code=r.status,d=null;
        if((/json/).test(r.headers.get('content-type')||'')){try{d=JSON.parse(t);}catch(e){d=null;}}
        else d=t;
        if(d&&typeof d==='string'&&d.trim().charAt(0)==='<'){res({error:true,code:code,body:'risposta HTML (endpoint mancante)'});return;}
        if(!r.ok){res({error:true,code:code,body:(d&&d.error)?d.error:t||('HTTP '+code)});return;}
        res(d===null?{error:true,code:code,body:'JSON non valido'}:d);
      });
    }).catch(function(e){res({error:true,code:0,body:(e&&e.message)?e.message:'net' });});
  });
}
function role(o){if(o&&o.role){if(S.role!==o.role){S.role=o.role;buildHomeMenu();}}}
function buildHomeMenu(){
  if($('mSec'))$('mSec').classList.toggle('hidden',S.role!=='owner');
  if($('mSet'))$('mSet').classList.toggle('hidden',S.role!=='owner');
}
function setNet(ok){
  var d=$('homeDot'),t=$('homeDotTxt');
  if(!d)return;
  d.className='ddot '+(ok?'on':'off');
  t.textContent=ok?'ONLINE':'OFFLINE';
}
function showError(code,msg,origin){
  S.retry=origin||'home';
  $('errCode').textContent='ESP8266 ha risposto: '+(code?code:'errore di rete');
  var d=(S.role==='owner'&&msg)?msg:'';
  $('errDetail').textContent=d;
  $('errDetail').classList.toggle('hidden',!d);
  go('err');
}
function renderHome(){
  $('homeCard').innerHTML=
    '<div class="row"><b>Device ID</b><span>'+esc(S.did)+'</span></div>'+
    '<div class="row"><b>Session</b><span>'+(S.role||'guest')+'</span></div>'+
    '<div class="row"><b>Modules</b><span>'+S.mods+'</span></div>'+
    '<div class="row"><b>Alerts</b><span>'+S.alerts+'</span></div>';
  $('homeAlert').classList.toggle('hidden',!(S.role==='owner'&&S.alerts>0));
  buildHomeMenu();
}
function go(id){
  S.screen=id;
  var all=document.querySelectorAll('.screen');
  for(var i=0;i<all.length;i++)all[i].classList.add('hidden');
  $('scr-'+id).classList.remove('hidden');
  if(id==='home')startPoll();else stopPoll();
  var f=RT[id];
  if(f)f();
}
var RT={
  home:loadHome,
  info:loadInfo,
  auth:loadAuth,
  funcs:loadFuncs,
  robot:loadRobot,
  wifi:loadWifi,
  wres:loadWres,
  sys:loadSys,
  ctl:loadCtl,
  camera:loadCamera,
  sec:loadSec,
  set:loadSet,
  net:loadNet,
  att:loadAtt,
  mon:loadMon,
  evil:loadEvil
};
function startPoll(){if(S.PH)return;stopPoll();S.PH=setInterval(pollHome,2000);pollHome();}
function stopPoll(){if(S.PH){clearInterval(S.PH);S.PH=null;}}
function pollHome(){
  api('/api/auth/status').then(function(r){
    var ok=!r.error;
    setNet(ok);
    if(ok){role(r);S.auth=r.authenticated?1:0;}
    if(ok&&(++S.HT)%6===0)loadStats();
  });
}
async function loadHome(){
  var d=await api('/api/device');
  var a=await api('/api/auth/status');
  if(a.error){setNet(false);}else{setNet(true);role(a);}
  if(!d.error){role(d);if(d.id)S.did=d.id;}
  await loadStats();
}
async function loadStats(){
  var m=await api('/api/modules');
  var cnt=0;
  if(!m.error){var mj=m;
    if(Array.isArray(mj)){for(var i=0;i<mj.length;i++)if(mj[i]&&mj[i].perm===true)cnt++;}
    else if(mj){for(var k in mj){if(mj[k]&&mj[k].perm===true)cnt++;}}}
  S.mods=cnt;
  if(S.role==='owner'){
    var s=await api('/api/security/status');
    if(!s.error){var sj=s;var al=sj.alertsList||sj.alerts||[];S.alerts=(typeof sj.count==='number')?sj.count:al.length;}
    else S.alerts=0;
  } else S.alerts=0;
  renderHome();
}
document.addEventListener('click',function(e){
  var t=e.target;
  while(t&&t!==document){
    var d=t.dataset;
    if(!d)return;
    if(d.g){go(d.g);return;}
    if(d.em){setEmotion(d.em);return;}
    if(d.anim){setAnim(d.anim);return;}
    if(d.tog){if(d.tog==='sleep')togSleep();else togBreath();return;}
    if(d.act){
      var a=d.act;
      if(a==='devlogin')devLogin();
      else if(a==='logout')doLogout2();
      else if(a==='scan')scanWifi();
      else if(a==='send')sendCtrl();
      else if(a==='refresh')loadSec();
      else if(a==='netscan')netScan();
      else if(a==='attackone'){if(t.dataset&&t.dataset.bssid)preAtt(t.dataset.bssid,t.dataset.ch,null);attackOne();go('att');}
      else if(a==='attackall')attackAll();
      else if(a==='attackbeacon')attackBeacon();
      else if(a==='attackstop')attackStop();
      else if(a==='monon')monOn();
      else if(a==='monoff')monOff();
      else if(a==='wdon')wdSet(true);
      else if(a==='wdoff')wdSet(false);
      else if(a==='wddl')wdDownload();
      else if(a==='wdclear')wdClear();
      else if(a==='evilgo')evilStart();
      else if(a==='evilstop')evilStop();
      else if(a==='evilrow'){if(t.dataset.ssid)$('evSsid').value=t.dataset.ssid;if(t.dataset.ch)$('evCh').value=t.dataset.ch;go('evil');}
      else if(a==='attackprobe')attackProbe();
      else if(a==='attackrnd')attackBeaconRnd();
      else if(a==='camerastart')cameraStart();
      else if(a==='camerastop')cameraStop();
      else if(a==='retry')go(S.retry);
      return;
    }
    t=t.parentNode;
  }
});
async function loadInfo(){
  var r=await Promise.all([api('/api/device'),api('/api/system'),api('/api/wifi/status')]);
  for(var i=0;i<r.length;i++){if(r[i].error){showError(r[i].code,r[i].body,'info');return;}}
  var d=r[0],ss=r[1],w=r[2];
  role(d);role(ss);role(w);
  S.upBase=ss.uptimeSec|0;S.upAt=Date.now();
  $('infoBody').innerHTML=
    '<div class="row"><b>Device ID</b><span>'+esc(d.id)+'</span></div>'+
    '<div class="row"><b>Nome</b><span>'+esc(d.name)+'</span></div>'+
    '<div class="row"><b>Firmware</b><span>'+esc(d.firmware)+'</span></div>'+
    '<div class="row"><b>Chip ID</b><span>'+esc(d.chipId)+'</span></div>'+
    '<div class="cap">Wi-Fi</div>'+
    '<div class="row"><b>Modalità</b><span>'+esc(w.mode||'AP')+'</span></div>'+
    '<div class="row"><b>SSID</b><span>'+esc(w.ssid)+'</span></div>'+
    '<div class="row"><b>IP</b><span>'+esc(w.ip)+'</span></div>'+
    '<div class="row"><b>Canale</b><span>'+esc(w.channel||'—')+'</span></div>'+
    '<div class="row"><b>Clienti</b><span>'+esc(w.clients!=null?w.clients:'—')+'</span></div>'+
    '<div class="cap">Sistema</div>'+
    '<div class="row"><b>Uptime</b><span id="infoUp">'+fmtUpt(S.upBase)+'</span></div>'+
    '<div class="row"><b>Memoria libera</b><span>'+esc(ss.freeHeap)+' B</span></div>'+
    '<div class="row"><b>FPS robot</b><span>'+esc(ss.robotFps)+'</span></div>';
}
async function loadAuth(){
  var r=await api('/api/auth/status');
  if(r.error){showError(r.code,r.body,'auth');return;}
  role(r);
  S.auth=r.authenticated?1:0;
  S.left=(r.sessionSecondsLeft!=null?r.sessionSecondsLeft:r.sessionLeftSec||0)|0;
  S.cid=r.clientId||'—';
  renderAuth();
}
function renderAuth(){
  var h='<div class="row"><b>Status</b><span>'+(S.auth?'🟢 AUTHENTICATED':'🔴 NOT AUTHENTICATED')+'</span></div>';
  if(S.auth)h+='<div class="row"><b>Client ID</b><span>'+esc(S.cid)+'</span></div>';
  h+='<div class="row"><b>Role</b><span>'+(S.role||'guest')+'</span></div>'+
     '<div class="row"><b>Session</b><span>'+(S.auth?(S.left+' s'):'—')+'</span></div>';
  if(S.role==='owner')h+='<div class="row"><b>OWNER CENTER</b><span>accesso consentito</span></div>';
  if(S.auth){
    h+='<div style="margin-top:12px"><button class="btn danger" style="width:100%" data-act="logout">🚪 LOGOUT</button></div>';
  }else{
    h+='<div class="cap">Accesso DEV</div>'+
      '<button class="btn" style="width:100%" data-act="devlogin">🔵 AUTENTICA (DEV)</button>'+
      '<div class="note">Face ID reale: in futuro, token firmato da iOS.</div>';
  }
  $('authBody').innerHTML=h;
}
function tickAuth(){
  if(S.auth>0){
    S.left--;if(S.left<1){S.left=0;S.auth=0;}
    renderAuth();
  }
}
function devLogin(){
  api('/api/auth/login',{body:JSON.stringify({mode:'dev',result:true})}).then(function(r){
    if(r.error){showError(r.code,r.body,'auth');return;}
    S.auth=r.authenticated?1:0;
    S.left=(r.sessionSecondsLeft!=null?r.sessionSecondsLeft:0)|0;
    role(r);
    renderAuth();
  });
}
function doLogout2(){
  api('/api/auth/logout',{method:'POST'}).then(function(){
    S.auth=0;S.left=0;renderAuth();
  });
}
function normMods(m){
  var MAP={robot:'robot',robotEyes:'robot',controls:'ctl',wifi:'wifi',wifi_scan:'wifi',scanner:'wifi',camera:'camera',net:'net',attack:'att',monitor:'mon',evil:'evil',wardrive:'net',system:'sys'};
  var r=[],i,k,id;
  if(Array.isArray(m)){for(i=0;i<m.length;i++){var it=m[i];
    if(it&&it.perm===true){id=it.key||it.id||it.name;if(MAP[id])r.push({k:MAP[id],n:it.name||id});}}}
  else if(m){for(k in m){var v=m[k];
    if(v&&v.perm===true&&MAP[k])r.push({k:MAP[k],n:v.name||k});}}
  return r;
}
async function loadFuncs(){
  var r=await api('/api/modules');
  if(r.error){showError(r.code,r.body,'funcs');return;}
  var m=r;role(m);
  var rows=normMods(m),h='';
  if(!rows.length)h='<div class="card"><div class="note">Nessun modulo autorizzato.</div></div>';
  for(var i=0;i<rows.length;i++)h+='<button class="big" data-g="'+rows[i].k+'">'+esc(rows[i].n)+'<span class="arr">›</span></button>';
  $('funcsBody').innerHTML=h;
}
var EMn=['neutral','happy','angry','sad','sleepy','surprised','curious','scared','love','laughing','thinking','wink_left','wink_right'];
var EMe=['😶','😀','😠','😢','😴','😮','🧐','😨','🥰','😂','🤔','😉','😉'];
async function loadRobot(){
  var r=await api('/api/system');
  if(r.error){showError(r.code,r.body,'robot');return;}
  role(r);S.fps=r.robotFps|0;
  var st=await api('/api/robot',{body:JSON.stringify({action:'state'})});
  if(!st.error){if(st.emotion!=null)S.em=st.emotion;if(st.fps!=null)S.fps=st.fps;S.sleep=!!st.sleep;}
  renderRobot();
}
function renderRobot(){
  var h='',i;
  for(i=0;i<EMn.length;i++){h+='<button '+(EMn[i]===S.em?'class="sel"':'')+' data-em="'+EMn[i]+'">'+EMe[i]+'<br>'+EMn[i]+'</button>';}
  $('robotEmo').innerHTML=h;
  $('robotAnim').innerHTML=
    '<button data-anim="blink">👁 Blink</button>'+
    '<button data-anim="double">👁 Doppio</button>'+
    '<button data-anim="wink_left">😉 Wink L</button>';
  $('robotTog').innerHTML=
    '<button data-tog="sleep">😴 Sleep '+(S.sleep?'ON':'OFF')+'</button>'+
    '<button data-tog="breath">💨 Breath '+(S.breath?'ON':'OFF')+'</button>';
  line();
}
function line(){$('robotLine').textContent='Occhi: '+S.em+' • FPS: '+S.fps;}
function postRob(o,cb){
  api('/api/robot',{body:JSON.stringify(o)}).then(function(r){
    if(r.error){$('robotMsg').textContent='⛔ ESP '+r.code+' '+r.body;return;}
    $('robotMsg').textContent=r.ok===false?'⛔ '+(r.error||'errore'):'✅ ok';
    if(cb)cb(r);
    if(r.emotion!=null)S.em=r.emotion;
    if(r.fps!=null)S.fps=r.fps;
    line();
  });
}
function setEmotion(v){postRob({action:'emotion',value:v});}
function setAnim(v){postRob({action:'anim',value:v});}
function togSleep(){postRob({action:'sleep',value:S.sleep?'off':'on'},function(j){if(j&&j.sleep!=null)S.sleep=j.sleep;renderRobot();});}
function togBreath(){S.breath=!S.breath;postRob({action:'breath',value:S.breath?'on':'off'});renderRobot();}
function renderCamera(r){
  if(!r)return;
  S.camera=r;
  $('camState').textContent=r.state||'IDLE';
  $('camSession').textContent=r.sessionId||'-';
  $('camExpiry').textContent=r.expiresInSec?r.expiresInSec+' s':'-';
  $('camStart').classList.toggle('hidden',r.state==='WAITING'||r.state==='SCANNED'||r.state==='CONNECTED');
  $('camStop').classList.toggle('hidden',!(r.state==='WAITING'||r.state==='SCANNED'||r.state==='REQUESTED'||r.state==='CONNECTED'));
  var m='';
  if(r.url){
    m='SCAN URL: '+esc(r.url)+
      '<a class="btn" style="display:block;text-align:center;text-decoration:none;margin-top:10px" href="'+esc(r.url)+'" target="_blank" rel="noopener norel">APRI CAMERA PAGE ›</a>';
  }
  if(r.hasLocation&&r.location&&r.location.lat){
    m+='<div class="note" style="margin-top:10px">POSITION RECEIVED<br>lat '+esc(r.location.lat)+
       '<br>lon '+esc(r.location.lon)+
       (r.location.accuracy?'<br>precision ±'+esc(r.location.accuracy):'')+'</div>';
  }
  $('camMsg').innerHTML=m;
}
function loadCamera(){
  api('/api/device/camera/status').then(function(r){
    if(r.error){$('camMsg').textContent='ESP '+r.code+' '+r.body;return;}
    renderCamera(r);
    var st=String(r.state||'');
    if(S.screen==='camera'&&(st==='WAITING'||st==='SCANNED'||st==='REQUESTED'||st==='CONNECTED')){
      setTimeout(loadCamera,2000);
    }
  });
}
function cameraStart(){
  $('camMsg').textContent='Creating temporary session...';
  api('/api/device/camera/start',{body:'{}'}).then(function(r){
    if(r.error){$('camMsg').textContent='ESP '+r.code+' '+r.body;return;}
    renderCamera(r);
  });
}
function cameraStop(){
  api('/api/device/camera/stop',{body:'{}'}).then(function(r){
    if(r.error){$('camMsg').textContent='ESP '+r.code+' '+r.body;return;}
    renderCamera(r);
  });
}
function loadWifi(){var btn=$('scanBtn');if(btn)btn.disabled=false;}
function scanWifi(){
  var btn=$('scanBtn');
  btn.disabled=true;btn.textContent='⏳ SCANSIONE...';
  api('/api/wifi/scan').then(function(r){
    btn.disabled=false;btn.textContent='📡 SCAN WIFI';
    if(r.error){showError(r.code,r.body,'wifi');return;}
    var j=jget(r)||r,st=String(j.mode||'').toLowerCase();
    var okr=['scanning','scan'].indexOf(st)>=0;
    if(okr){S.decoy=(j.mode==='decoy');S.nets=(j.networks&&j.networks.length)?j.networks:[];go('wres');}
    else{showError(r.code||st||'failed',JSON.stringify(j),'wifi');return;}
  });
}
function riskCls(risk){
  risk=String(risk||'unknown').toLowerCase();
  if(risk==='good')return'good';
  if(risk==='normal'||risk==='ok'||risk==='low')return'norm';
  if(risk==='high')return'high';
  if(risk==='critical')return'crit';
  return'unk';
}
function wresRow(nt){
  return '<div class="net"><div class="n">'+(esc(nt.ssid)||'(nascosta)')+'</div>'+
    '<div class="m">'+(nt.bssid?esc(nt.bssid)+' • ':'')+'CH '+(nt.channel!=null?esc(nt.channel):'?')+
    ' • '+(nt.rssi!=null?esc(nt.rssi)+' dBm':'?')+((nt.security!=null&&nt.security!=='')?' • '+esc(nt.security):'')+'</div>'+
    '<span class="risk '+riskCls(nt.risk)+'">'+(nt.risk!=null?esc(nt.risk):'unknown')+'</span>'+
    (nt.note?' <span class="note">'+esc(nt.note)+'</span>':'')+'</div>';
}
function loadWres(){
  var h='';
  if(S.decoy&&S.role==='owner')h+='<div class="note" style="margin:12px">📋 (simulazione DECOY)</div>';
  if(!S.nets.length){h+='<div class="card"><div class="note">Nessuna rete trovata.</div></div>';}
  else{for(var i=0;i<S.nets.length;i++)h+=wresRow(S.nets[i]);}
  $('wresBody').innerHTML=h;
}
async function loadSys(){
  var r=await api('/api/system');
  if(r.error){showError(r.code,r.body,'sys');return;}
  role(r);
  S.upBase=r.uptimeSec|0;S.upAt=Date.now();
  $('sysBody').innerHTML=
    '<div class="row"><b>Device</b><span>'+esc(r.device)+'</span></div>'+
    '<div class="row"><b>Uptime</b><span id="sysUp">'+fmtUpt(S.upBase)+'</span></div>'+
    '<div class="row"><b>Memoria libera</b><span>'+esc(r.freeHeap)+' B</span></div>'+
    '<div class="row"><b>Sketch libera</b><span>'+esc(r.freeSketch)+' B</span></div>'+
    '<div class="row"><b>Reset reason</b><span>'+esc(r.resetReason)+'</span></div>'+
    '<div class="row"><b>FPS robot</b><span>'+esc(r.robotFps)+'</span></div>';
}
async function loadCtl(){
  var r=await api('/api/values');
  if(r.error){showError(r.code,r.body,'ctl');return;}
  var v=Array.isArray(r)?r:jget(r),h='';
  if(!Array.isArray(v)||!v.length)h='<div class="note">Nessun valore registrato.</div>';
  else{for(var i=0;i<v.length;i++)h+='<div class="row"><b>'+esc(v[i].name)+'</b><span>'+esc(v[i].value)+'</span></div>';}
  $('ctlList').innerHTML=h;
}
function sendCtrl(){
  var n=$('cName').value.trim(),vv=$('cVal').value;
  if(!n){$('ctlMsg').textContent='nome mancante';return;}
  api('/api/control',{body:JSON.stringify({name:n,value:vv})}).then(function(r){
    $('ctlMsg').textContent=r.error?('⛔ ESP '+r.code+' '+esc(r.body)):'✅ inviato: '+esc(n)+' = '+esc(vv);
    if(!r.error)loadCtl();
  });
}
function devRows(list,label){
  var h='<div class="row"><b>'+label+'</b><span>'+(list?list.length:0)+'</span></div>';
  if(list){for(var i=0;i<list.length;i++){var d=list[i];
    h+='<div class="row"><b>'+esc(d.mac||d.id||d.name||d.ip||'?')+'</b><span>'+esc(d.ip||'')+'</span></div>';
  }}
  return h;
}
async function loadSec(){
  var s=await api('/api/security/status');
  if(s.error){showError(s.code,s.body,'sec');return;}
  var sj=s;role(sj);
  var al=(sj&&(sj.alertsList||sj.alerts))||[];
  var cnt=(sj&&typeof sj.count==='number')?sj.count:al.length;
  var ah='<div class="row"><b>Alert attivi</b><span>'+cnt+'</span></div>';
  if(!al.length)ah+='<div class="note">Nessun alert.</div>';
  else for(var i=0;i<al.length;i++)ah+='<div class="row"><b>⚠ '+esc(typeof al[i]==='string'?'ALERT':(al[i].level||'ALERT'))+'</b><span>'+esc(typeof al[i]==='string'?al[i]:(al[i].msg||al[i].text||''))+'</span></div>';
  $('secAlerts').innerHTML=ah;
  var au=await api('/api/security/audit');
  var at='';
  if(au.error)at='<div class="note">Audit non disponibile ('+au.code+')</div>';
  else{var aj=jget(au);var rows=Array.isArray(aj)?aj:((aj&&aj.audit)||(aj&&aj.entries)||(aj&&aj.log)||[]);
    at='<table><tr><th>t</th><th>client</th><th>func</th><th>allowed</th><th>decoy</th></tr>';
    for(var i2=0;i2<rows.length;i2++){var o=rows[i2];if(!o)continue;
      at+='<tr><td>'+esc(o.t)+'</td><td>'+esc(o.client)+'</td><td>'+esc(o.func)+'</td><td>'+(o.allowed?'✅':'⛔')+'</td><td>'+(o.decoy?'📋':'')+'</td></tr>';}
    at+='</table>';}
  $('secAudit').innerHTML=at;
  var dv=await api('/api/security/devices');
  var dt='';
  if(dv.error)dt='<div class="note">Device non disponibili ('+dv.code+')</div>';
  else{var dj=dv;
    dt=devRows(dj.clients,'Collegati')+devRows(dj.unknown,'Sconosciuti');}
  $('secDev').innerHTML=dt;
}
async function loadSet(){
  var r=await api('/api/settings');
  if(r.error){showError(r.code,r.body,'set');return;}
  role(r);
  $('setBody').innerHTML=
    '<div class="row"><b>Nome</b><span>'+esc(r.name)+'</span></div>'+
    '<div class="row"><b>Firmware</b><span>'+esc(r.firmware)+'</span></div>'+
    '<div class="row"><b>Auth mode</b><span>'+esc(r.authMode)+'</span></div>'+
    '<div class="row"><b>Session timeout</b><span>'+esc(r.sessionTimeoutSec)+' s</span></div>'+
    '<div class="row"><b>Wi-Fi Scan</b><span>'+esc(r.wifiScan)+'</span></div>';
}
function preAtt(bssid,ch,ssid){if(bssid)$('attBssid').value=bssid;if(ch)$('attCh').value=ch;if(ssid)$('attSsid').value=ssid;}
function attMsg(t){var el=$('attMode');if(el)el.textContent=t;}
function doAttack(type,body){
  api('/api/net/attack',{body:JSON.stringify(body)}).then(function(r){
    if(r.error){attMsg('⛔ '+r.code+' '+r.body);return;}
    attMsg(r.ok===false?(r.error||'errore'):'✅ '+String(r.mode||type));
    loadAtt();
  });
}
function attackOne(){
  var b=($('attBssid').value||'').trim(),c=($('attCh').value||'').trim();
  if(!b){attMsg('⚠ inserisci un BSSID');return;}
  doAttack('deauth',{type:'deauth',bssid:b,channel:(c?parseInt(c,10):0),timeout:60});
}
function attackAll(){doAttack('deauthall',{type:'deauthall',timeout:60});}
function attackBeacon(){var s=($('attSsid').value||'').trim();doAttack('beacon',{type:'beacon',ssid:s,timeout:60});}
function attackProbe(){var s=($('attSsid2').value||'').trim();doAttack('probe',{type:'probe',ssid:s,timeout:60});}
function attackBeaconRnd(){doAttack('beaconrnd',{type:'beaconrnd',timeout:60});}
function attackStop(){
  api('/api/net/attack/stop').then(function(r){if(!r.error)attMsg('🛑 fermato');else attMsg('⛔ '+r.body);loadAtt();});
}
async function loadAtt(){
  var r=await api('/api/net/attack');
  if(r.error){attMsg('⛔ '+r.code+' '+r.body);return;}
  function set(id,v){var el=$(id);if(el)el.textContent=v;}
  set('attMode',String(r.mode||'idle'));
  set('attTarget',r.target||'—');
  set('attSent',String(r.sent!=null?r.sent:0));
  set('attPps',String(r.pps!=null?r.pps:0));
  if(r.remaining==null||r.remaining<0)set('attRem','∞');else set('attRem',(r.remaining|0)+' s');
  if(r.running)setTimeout(loadAtt,1200);
}
function renderNetNote(t){var el=$('netNote');if(el)el.textContent=t;}
function applyNetData(r){
  S.nets2=(r.networks&&r.networks.length)?r.networks:[];
  S.stas=(r.stations&&r.stations.length)?r.stations:[];
  renderNet();
}
function renderNet(){
  var h='',i;
  if(S.mode2==='scanning')h+='<div class="note">⏳ scansione in corso...</div>';
  if(!S.nets2.length){h+='<div class="note">Nessuna rete. Avvia lo scan profondo.</div>';}
  else for(i=0;i<S.nets2.length;i++){var nth=S.nets2[i];
    h+='<div class="net"><div class="n">'+esc(nth.ssid||'(nascosta)')+'</div>'+
       '<div class="m">'+esc(nth.bssid||'')+' • CH '+(nth.channel!=null?esc(nth.channel):'?')+
       ' • '+(nth.rssi!=null?esc(nth.rssi)+' dBm':'?')+
       ' • '+esc(nth.security||'?')+' • clienti: '+(nth.clients!=null?nth.clients:0)+'</div>'+
       '<span class="risk '+riskCls(nth.risk)+'">'+(nth.risk!=null?esc(nth.risk):'unknown')+'</span> '+
       '<button class="btn" style="margin-top:8px;width:100%" data-bssid="'+esc(nth.bssid||'')+'" data-ch="'+(nth.channel||'')+'" data-act="attackone">⚡ DEAUTH '+esc(nth.ssid||'')+'</button>'+
       '<button class="btn danger" style="margin-top:6px;width:100%" data-ssid="'+esc(nth.ssid||'')+'" data-ch="'+(nth.channel||'')+'" data-act="evilrow">🎭 EVIL TWIN '+esc(nth.ssid||'')+'</button></div>';
  }
  if(S.stas.length){
    h+='<div class="cap" style="margin-top:14px">Client rilevati</div>';
    for(i=0;i<S.stas.length;i++){var sn=S.stas[i];
      h+='<div class="row"><b>'+esc(sn.mac)+'</b><span>CH '+(sn.channel!=null?esc(sn.channel):'?')+' • AP#'+(sn.ap!=null?esc(sn.ap):'?')+'</span></div>';}
  }
  $('netList').innerHTML=h;
}
function netScan(){
  var btn=$('netScanBtn');
  if(btn){btn.disabled=true;btn.textContent='⏳ SCANSIONE...';
  api('/api/wifi/scan').then(function(r){
    if(btn){btn.disabled=false;btn.textContent='📡 SCAN PROFONDO';
    if(r.error){renderNetNote('⛔ '+r.code+' '+r.body);return;}
    S.mode2='scanning';renderNetNote('⏳ scansione in corso...');renderNet();
    setTimeout(netPoll,1500);
  });
}
function netPoll(){
  api('/api/net/scan').then(function(r){
    if(r.error){S.mode2='';renderNetNote('⛔ '+r.code+' '+r.body);renderNet();return;}
    applyNetData(r);
    var m=String(r.mode||'').toLowerCase();
    var running=r.running||false;
    if(m==='scanning'||m==='scan'||running){setTimeout(netPoll,1500);}
    else{S.mode2='';renderNetNote('Scan completato: '+S.nets2.length+' reti.');}
  });
}
async function loadNet(){
  renderNetNote('Elenco reti + client associati.');
  loadWD();
  var r=await api('/api/net/scan');
  if(r.error){renderNetNote('⛔ '+r.code+' '+r.body);return;}
  applyNetData(r);
  var m=String(r.mode||'').toLowerCase();
  var running=r.running||false;
  if(m==='scanning'||m==='scan'||running){S.mode2='scanning';setTimeout(netPoll,1500);}else S.mode2='';
}
function monOn(){api('/api/net/sniff?on=1').then(function(r){if(r.error)showError(r.code,r.body,'mon');else loadMon();});}
function monOff(){api('/api/net/sniff?on=0').then(function(r){if(r.error)showError(r.code,r.body,'mon');else loadMon();});}
function wdSet(on){api('/api/wardrive?on='+(on?'1':'0')).then(function(r){if(r.error)showError(r.code,r.body,'net');else renderWD(r);});}
function wdClear(){api('/api/wardrive?clear=1').then(function(r){if(r.error)showError(r.code,r.body,'net');else renderWD(r);});}
function wdDownload(){window.location='/download';}
function renderWD(r){
  var el=$('netWD');if(!el)return;
  el.textContent=
    (r.enabled?'🟢 ON':'⚪ OFF')+
    ' • reti: '+((r.entries!=null)?r.entries:0)+
    ' • '+(r.size!=null?fmtBytes(r.size):'')+
    (r.fs?'':' • FS OU');
}
function fmtBytes(b){b=b|0;if(b>1048576)return (b/1048576).toFixed(1)+' MB';if(b>1024)return (b/1024).toFixed(1)+' KB';return b+' B';}
function loadWD(){api('/api/wardrive').then(function(r){if(!r.error)renderWD(r);});}
function evilMsg(t){var el=$('evilStatus');if(el)el.innerHTML=t;}
function evilStart(){
  var s=($('evSsid').value||'').trim(),c=($('evCh').value||'').trim();
  if(!s){evilMsg('<div class="note">⚠ inserisci l\'SSID da clonare</div>');return;}
  api('/api/evil',{body:JSON.stringify({ssid:s,channel:(c?parseInt(c,10):1)})}).then(function(r){
    if(r.error){evilMsg('<div class="note">⛔ '+r.code+' '+esc(r.body)+'</div>');return;}
    evilMsg('<div class="note">✅ Clone avviato: '+esc(r.ssid)+'</div>');
    loadEvil();
  });
}
function evilStop(){api('/api/evil/stop').then(function(r){if(!r.error){evilMsg('<div class="note">🛑 fermato, AP ripristinato.</div>');loadEvil();}else evilMsg('<div class="note">⛔ '+esc(r.body)+'</div>');});}
async function loadEvil(){
  var r=await api('/api/evil');
  if(r.error){evilMsg('<div class="note">⛔ '+esc(r.code)+' '+esc(r.body)+'</div>');return;}
  var h='<div class="row"><b>Stato</b><span>'+(r.running?'🎭 ATTIVO':'idle')+'</span></div>'+
        '<div class="row"><b>SSID falso</b><span>'+esc(r.ssid||'—')+'</span></div>'+
        '<div class="row"><b>Canale</b><span>'+esc(r.channel!=null?r.channel:'—')+'</span></div>'+
        '<div class="row"><b>Password catturate</b><span>'+esc(r.captured!=null?r.captured:0)+'</span></div>';
  var cc=(r.creds&&r.creds.length)?r.creds:[];
  if(cc.length){h+='<div class="cap" style="margin-top:10px">Credenziali raccolte</div>';
    for(var i=0;i<cc.length;i++)h+='<div class="row"><b>'+esc(cc[i].ssid||'')+'</b><span>'+esc(cc[i].pass||'')+'</span></div>';}
  evilMsg(h);
  if(r.running)setTimeout(loadEvil,1500);
}
function loadMon(){
  api('/api/net/sniff').then(function(r){
    if(r.error)return;
    var btn=$('monBtn');
    if(r.on){
      if(btn){btn.textContent='🐾 STOP MONITOR';btn.dataset.act='monoff';}
      $('monStats').innerHTML=
        '<div class="row"><b>Canale</b><span>'+(r.channel!=null?esc(r.channel):'?')+'</span></div>'+
        '<div class="row"><b>Pacchetti</b><span>'+esc(r.packets)+'</span></div>'+
        '<div class="row"><b>Deauth osservati</b><span>'+esc(r.deauthRx)+'</span></div>'+
        '<div class="row"><b>Beacon</b><span>'+esc(r.beaconRx)+'</span></div>'+
        '<div class="row"><b>Probe</b><span>'+esc(r.probeRx)+'</span></div>'+
        '<div class="row"><b>Data</b><span>'+esc(r.dataRx)+'</span></div>'+
        '<div class="row"><b>Auto stop</b><span>'+(r.remaining!=null?esc(r.remaining)+' s':'—')+'</span></div>';
      var hs='',stas=(r.stations&&r.stations.length)?r.stations:[];
      if(!stas.length)hs='<div class="note">Nessun client rilevato.</div>';
      else for(var i=0;i<stas.length;i++)hs+='<div class="row"><b>'+esc(stas[i].mac)+'</b><span>CH '+(stas[i].channel!=null?esc(stas[i].channel):'?')+' • AP#'+(stas[i].ap!=null?esc(stas[i].ap):'?')+'</span></div>';
      $('monSta').innerHTML=hs;
      var hp='',pr=(r.probes&&r.probes.length)?r.probes:[];
      if(!pr.length)hp='<div class="note">Nessuna probe request osservata.</div>';
      else for(var j=0;j<pr.length;j++)hp+='<div class="row"><b>'+esc(pr[j].mac)+'</b><span>📡 cerca "'+esc(pr[j].ssid||'')+'"</span></div>';
      $('monProbe').innerHTML=hp;
      setTimeout(loadMon,1200);
    }else{
      if(btn){btn.textContent='🐾 AVVIA MONITOR';btn.dataset.act='monon';}
      $('monStats').innerHTML='<div class="note">Monitor spento.</div>';
      $('monSta').innerHTML='';
      $('monProbe').innerHTML='';
    }
  });
}
function tickUp(){
  var t=fmtUpt(S.upBase+(Date.now()-S.upAt)/1000);
  var a=$('infoUp');if(a)a.textContent=t;
  var b=$('sysUp');if(b)b.textContent=t;
}
setInterval(function(){
  if(S.screen==='info'||S.screen==='sys')tickUp();
  else if(S.screen==='auth')tickAuth();
},1000);
go('home');
</script>
</body>
</html>
)rawlit";

const char* page() {
    return PAGE;
}

String html() {
    return String(FPSTR(PAGE));
}

} // namespace dashboard
