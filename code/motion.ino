#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <SPIFFS.h>

const char* ssid     = "wifi_name";
const char* password = "wifi_pass";

#define PIR_PIN 13
#define MIN_HOLD_MS 500   // shorter than this = false positive (electrical noise)

WebServer server(80);
WebSocketsServer ws(81);
volatile bool motionFlag = false;
volatile bool clearFlag = false;
int totalReal = 0;
int totalFalse = 0;
unsigned long motionStartTime = 0;
unsigned long bootTime = 0;
bool warmedUp = false;
bool motionActive = false;

void IRAM_ATTR motionISR(){
  if(digitalRead(PIR_PIN)==HIGH) motionFlag=true;
  else clearFlag=true;
}

void writeLog(String msg){
  File f = SPIFFS.open("/log.txt", FILE_APPEND);
  if(f){ f.println(msg); f.close(); }
}

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Motion Alert</title>
<style>
*{box-sizing:border-box;margin:0;padding:0;-webkit-tap-highlight-color:transparent;}
body{font-family:-apple-system,system-ui,sans-serif;background:#000;color:#fff;min-height:100vh;}
.app{max-width:440px;margin:0 auto;padding:20px 16px 40px;}
.header{display:flex;justify-content:space-between;align-items:center;margin-bottom:20px;padding-top:8px;}
.title{font-size:28px;font-weight:700;letter-spacing:-0.5px;}
.live{display:flex;align-items:center;gap:6px;font-size:13px;font-weight:600;color:#30d158;background:#1c1c1e;border-radius:20px;padding:6px 12px;}
.live-dot{width:7px;height:7px;border-radius:50%;background:#30d158;animation:pulse 1.5s infinite;}
@keyframes pulse{0%,100%{opacity:1;transform:scale(1)}50%{opacity:.5;transform:scale(.85)}}
.seg{display:flex;background:#1c1c1e;border-radius:11px;padding:2px;margin-bottom:20px;}
.seg-btn{flex:1;padding:8px;border:none;font-size:14px;font-weight:600;cursor:pointer;background:transparent;color:#8e8e93;border-radius:9px;transition:all .2s;}
.seg-btn.active{background:#2c2c2e;color:#fff;}
.page{display:none;}
.page.active{display:block;}
.radar-card{background:#1c1c1e;border-radius:20px;padding:20px;margin-bottom:16px;display:flex;justify-content:center;transition:background .3s;}
.radar-card.alert{background:#2a1416;}
.radar-wrap{position:relative;width:260px;height:260px;}
.rsvg{width:260px;height:260px;}
.rcenter{position:absolute;top:50%;left:50%;transform:translate(-50%,-50%);text-align:center;pointer-events:none;width:160px;}
.rst{font-size:11px;color:#48484a;letter-spacing:.1em;margin-bottom:3px;font-weight:600;}
.rst.alert{color:#ff453a;}
.rcount{font-size:44px;font-weight:700;color:#fff;line-height:1;letter-spacing:-1px;}
.rlbl{font-size:11px;color:#48484a;margin-top:3px;}
.held-pill{display:inline-block;margin-top:10px;font-size:13px;font-weight:700;color:#8e8e93;background:#2c2c2e;border-radius:20px;padding:4px 12px;font-variant-numeric:tabular-nums;}
.held-pill.live{color:#ff453a;background:#3a1d1d;}
.stat-grid{display:grid;grid-template-columns:1fr 1fr;gap:12px;margin-bottom:16px;}
.stat-card{border-radius:16px;padding:18px;}
.stat-card.green{background:#30d158;}
.stat-card.orange{background:#ff9f0a;}
.stat-label{font-size:13px;font-weight:600;margin-bottom:6px;color:#fff;opacity:.9;}
.stat-num{font-size:34px;font-weight:700;letter-spacing:-1px;line-height:1;color:#fff;}
.section-label{font-size:13px;font-weight:600;color:#8e8e93;margin:0 4px 10px;text-transform:uppercase;letter-spacing:0.3px;}
.events{background:#1c1c1e;border-radius:16px;overflow:hidden;}
.event{display:flex;align-items:flex-start;gap:12px;padding:14px 16px;border-bottom:0.5px solid #2c2c2e;}
.event:last-child{border-bottom:none;}
.event.ongoing{background:#2a1416;}
.event-dot{width:10px;height:10px;border-radius:50%;flex-shrink:0;margin-top:4px;}
.event-dot.real{background:#ff453a;}
.event-dot.false{background:#ff9f0a;}
.event-dot.ongoing{background:#ff453a;animation:pulse 1s infinite;}
.event-body{flex:1;}
.event-title{font-size:15px;font-weight:600;color:#fff;}
.event-meta{font-size:13px;color:#8e8e93;margin-top:2px;line-height:1.4;}
.event-dur{font-size:13px;font-weight:600;color:#ff9f0a;font-variant-numeric:tabular-nums;}
.empty{padding:40px 20px;text-align:center;color:#48484a;font-size:15px;}
.info-card{background:#1c1c1e;border-radius:16px;padding:18px;margin-bottom:16px;}
.info-card-title{font-size:13px;font-weight:600;color:#8e8e93;text-transform:uppercase;letter-spacing:0.3px;margin-bottom:14px;}
.info-row{display:flex;justify-content:space-between;align-items:center;padding:8px 0;}
.info-row+.info-row{border-top:0.5px solid #2c2c2e;}
.info-key{font-size:14px;color:#8e8e93;}
.info-val{font-size:14px;font-weight:600;color:#fff;}
.info-hint{font-size:12px;color:#48484a;margin-top:12px;line-height:1.5;}
.log-toolbar{display:flex;gap:8px;margin-bottom:14px;}
.tool-btn{flex:1;padding:11px;border:none;border-radius:12px;background:#1c1c1e;color:#0a84ff;font-size:15px;font-weight:600;cursor:pointer;}
.tool-btn:active{background:#2c2c2e;}
.tool-btn.danger{color:#ff453a;}
.raw-card{background:#1c1c1e;border-radius:16px;padding:16px;font-family:'SF Mono',ui-monospace,monospace;font-size:12px;line-height:1.9;max-height:420px;overflow-y:auto;word-break:break-all;color:#8e8e93;}
.raw-card .ml{color:#ff453a;font-weight:600;}
.raw-card .cl{color:#30d158;}
.raw-card .fp{color:#ff9f0a;}
.raw-card .il{color:#48484a;}
.copy-hint{font-size:12px;color:#48484a;margin-top:10px;text-align:center;}
@keyframes spin{from{transform:rotate(0deg)}to{transform:rotate(360deg)}}
</style></head><body>
<div class="app">
  <div class="header">
    <span class="title">Motion</span>
    <span class="live"><span class="live-dot"></span>Live</span>
  </div>

  <div class="seg">
    <button class="seg-btn active" onclick="showTab('home')">Home</button>
    <button class="seg-btn" onclick="showTab('log')">Log</button>
  </div>

  <div class="page active" id="tab-home">
    <div class="radar-card" id="radar-card">
      <div class="radar-wrap">
        <svg class="rsvg" viewBox="0 0 260 260">
          <circle cx="130" cy="130" r="120" fill="none" stroke="#2c2c2e" stroke-width="1"/>
          <circle cx="130" cy="130" r="90" fill="none" stroke="#2c2c2e" stroke-width="1"/>
          <circle cx="130" cy="130" r="60" fill="none" stroke="#2c2c2e" stroke-width="1"/>
          <circle cx="130" cy="130" r="30" fill="none" stroke="#2c2c2e" stroke-width="1"/>
          <line x1="10" y1="130" x2="250" y2="130" stroke="#222224" stroke-width="0.8"/>
          <line x1="130" y1="10" x2="130" y2="250" stroke="#222224" stroke-width="0.8"/>
          <g id="sweep" style="transform-origin:130px 130px;animation:spin 4s linear infinite;">
            <path d="M130 130 L130 10 A120 120 0 0 1 215 45 Z" fill="#30d15814"/>
            <line x1="130" y1="130" x2="130" y2="10" stroke="#30d158" stroke-width="1.5" opacity="0.5"/>
          </g>
          <circle id="ping" cx="130" cy="130" r="0" fill="none" stroke="#ff453a" stroke-width="1.5" opacity="0"/>
          <circle id="mdot" cx="130" cy="130" r="6" fill="#ff453a" opacity="0" style="transition:opacity .2s;"/>
          <circle cx="130" cy="130" r="3.5" fill="#30d158"/>
        </svg>
        <div class="rcenter">
          <div class="rst" id="rst">SCANNING</div>
          <div class="rcount" id="rcount">0</div>
          <div class="rlbl">detections</div>
          <div class="held-pill" id="held-pill">ready</div>
        </div>
      </div>
    </div>

    <div class="stat-grid">
      <div class="stat-card green">
        <div class="stat-label">Real motion</div>
        <div class="stat-num" id="st-real">0</div>
      </div>
      <div class="stat-card orange">
        <div class="stat-label">False alarms</div>
        <div class="stat-num" id="st-false">0</div>
      </div>
    </div>

    <div class="section-label">Recent events</div>
    <div class="events" id="events">
      <div class="empty">No events yet</div>
    </div>
  </div>

  <div class="page" id="tab-log">
    <div class="info-card">
      <div class="info-card-title">Sensor info</div>
      <div class="info-row"><span class="info-key">Model</span><span class="info-val">HC-SR501 PIR</span></div>
      <div class="info-row"><span class="info-key">Detection range</span><span class="info-val">up to 7 m</span></div>
      <div class="info-row"><span class="info-key">Detection angle</span><span class="info-val">110&deg; cone</span></div>
      <div class="info-row"><span class="info-key">Measures</span><span class="info-val">motion only</span></div>
      <div class="info-hint">This sensor detects movement, not distance. "Held time" shown in events is how long someone stayed within range, not how far away they were.</div>
    </div>

    <div class="log-toolbar">
      <button class="tool-btn" onclick="loadLog()">Refresh</button>
      <button class="tool-btn" onclick="copyLog()">Copy all</button>
      <button class="tool-btn danger" onclick="clearLog()">Clear</button>
    </div>
    <div class="raw-card" id="rawlog"><div class="empty">Tap Refresh to load</div></div>
    <div class="copy-hint">Copy all, then paste to analyze your data</div>
  </div>
</div>

<script>
var realTotal=0,falseTotal=0,unlocked=false,hasEvents=false,ai=0;
var angles=[30,75,130,190,250,310,50,170,290,110,220];
var ongoingEl=null,ongoingStart=null,ongoingTimer=null;
var ctx=window.AudioContext?new AudioContext():window.webkitAudioContext?new webkitAudioContext():null;

document.body.addEventListener('click',function(){
  if(ctx&&ctx.state==='suspended')ctx.resume();
  unlocked=true;
});

function showTab(name){
  document.querySelectorAll('.seg-btn').forEach(function(b){
    b.classList.toggle('active',b.textContent.toLowerCase().indexOf(name)>=0);
  });
  document.getElementById('tab-home').classList.toggle('active',name==='home');
  document.getElementById('tab-log').classList.toggle('active',name==='log');
  if(name==='log')loadLog();
}

function chime(){
  if(!unlocked||!ctx)return;
  [[523,0],[659,0.12],[784,0.24]].forEach(function(n){
    setTimeout(function(){
      var o=ctx.createOscillator(),g=ctx.createGain();
      o.type='sine';o.connect(g);g.connect(ctx.destination);
      o.frequency.value=n[0];
      g.gain.setValueAtTime(0,ctx.currentTime);
      g.gain.linearRampToValueAtTime(0.16,ctx.currentTime+0.04);
      g.gain.exponentialRampToValueAtTime(0.001,ctx.currentTime+0.38);
      o.start();o.stop(ctx.currentTime+0.39);
    },n[1]*1000);
  });
}

function blip(){
  var angle=angles[ai%angles.length]*Math.PI/180; ai++;
  var dist=28+Math.random()*82;
  var cx=130+Math.sin(angle)*dist;
  var cy=130-Math.cos(angle)*dist;
  var dot=document.getElementById('mdot');
  dot.setAttribute('cx',cx);dot.setAttribute('cy',cy);
  dot.style.opacity='1';
  var p=document.getElementById('ping');
  p.setAttribute('cx',cx);p.setAttribute('cy',cy);
  var s=null;
  function anim(ts){
    if(!s)s=ts;var pr=(ts-s)/900;
    if(pr<1){p.setAttribute('r',6+pr*92);p.style.opacity=0.9*(1-pr);requestAnimationFrame(anim);}
    else{p.style.opacity='0';}
  }
  requestAnimationFrame(anim);
}

function fmtTime(d){return d.toLocaleTimeString([],{hour:'2-digit',minute:'2-digit',second:'2-digit'});}

// Called when motion STARTS — show ongoing event, start live timer
function startOngoing(){
  if(!hasEvents){document.getElementById('events').innerHTML='';hasEvents=true;}
  ongoingStart=Date.now();
  var startStr=fmtTime(new Date());

  var ev=document.createElement('div');
  ev.className='event ongoing';
  ev.innerHTML='<div class="event-dot ongoing"></div>'+
    '<div class="event-body"><div class="event-title">Motion in progress</div>'+
    '<div class="event-meta">Started '+startStr+'</div></div>'+
    '<div class="event-dur" id="live-dur">0.0s</div>';
  var box=document.getElementById('events');
  box.prepend(ev);
  ongoingEl=ev;

  // update the live duration every 100ms
  ongoingTimer=setInterval(function(){
    if(!ongoingStart)return;
    var sec=((Date.now()-ongoingStart)/1000).toFixed(1);
    var de=document.getElementById('live-dur');
    if(de)de.textContent=sec+'s';
    var pill=document.getElementById('held-pill');
    pill.textContent=sec+'s';
    pill.className='held-pill live';
  },100);
}

// Called when motion ENDS — finalize the event with total duration
function endOngoing(heldSec,isFalse){
  if(ongoingTimer){clearInterval(ongoingTimer);ongoingTimer=null;}
  var startStr=ongoingStart?fmtTime(new Date(ongoingStart)):'';
  var endStr=fmtTime(new Date());

  if(ongoingEl){
    ongoingEl.className='event';
    var dotClass=isFalse?'false':'real';
    var title=isFalse?'False alarm':'Motion detected';
    var meta='From '+startStr+' to '+endStr;
    if(isFalse) meta='Too brief - '+heldSec+'s';
    ongoingEl.innerHTML='<div class="event-dot '+dotClass+'"></div>'+
      '<div class="event-body"><div class="event-title">'+title+'</div>'+
      '<div class="event-meta">'+meta+'</div></div>'+
      '<div class="event-dur" style="color:'+(isFalse?'#ff9f0a':'#30d158')+'">'+heldSec+'s</div>';
    var box=document.getElementById('events');
    while(box.children.length>20)box.removeChild(box.lastChild);
  }
  ongoingEl=null;ongoingStart=null;
}

function loadLog(){
  var box=document.getElementById('rawlog');
  box.innerHTML='<div class="empty">Loading...</div>';
  fetch('/getlog').then(function(r){return r.text();}).then(function(txt){
    if(!txt||txt.trim()===''){box.innerHTML='<div class="empty">Log is empty</div>';return;}
    var lines=txt.trim().split('\n').reverse();
    var html='';
    lines.forEach(function(line){
      var cls='';
      if(line.indexOf('FALSE')>=0)cls=' class="fp"';
      else if(line.indexOf('MOTION')>=0)cls=' class="ml"';
      else if(line.indexOf('CLEAR')>=0)cls=' class="cl"';
      else if(line.indexOf('===')>=0)cls=' class="il"';
      html+='<div'+cls+'>'+line+'</div>';
    });
    box.innerHTML=html;
  }).catch(function(){box.innerHTML='<div class="empty">Could not load</div>';});
}

function copyLog(){
  var text=document.getElementById('rawlog').innerText;
  if(navigator.clipboard){
    navigator.clipboard.writeText(text).then(function(){alert('Copied! Paste to analyze.');});
  } else {
    var ta=document.createElement('textarea');ta.value=text;
    document.body.appendChild(ta);ta.select();
    document.execCommand('copy');document.body.removeChild(ta);alert('Copied!');
  }
}

function clearLog(){
  if(!confirm('Clear all log data?'))return;
  fetch('/clearlog').then(function(){
    document.getElementById('rawlog').innerHTML='<div class="empty">Log cleared</div>';
    realTotal=0;falseTotal=0;hasEvents=false;
    document.getElementById('rcount').textContent='0';
    document.getElementById('st-real').textContent='0';
    document.getElementById('st-false').textContent='0';
    document.getElementById('held-pill').textContent='ready';
    document.getElementById('held-pill').className='held-pill';
    document.getElementById('events').innerHTML='<div class="empty">No events yet</div>';
  });
}

var ws=new WebSocket('ws://'+location.hostname+':81/');
ws.onmessage=function(e){
  var d=JSON.parse(e.data);
  var card=document.getElementById('radar-card');
  var rst=document.getElementById('rst');
  var pill=document.getElementById('held-pill');

  if(d.type==='start'){
    // Motion just began — someone entered the zone
    card.className='radar-card alert';
    rst.textContent='DETECTING';
    rst.className='rst alert';
    blip();chime();
    startOngoing();

  } else if(d.type==='motion'){
    // Motion ended and it was real (long enough)
    var held=(d.held_ms/1000).toFixed(1);
    realTotal++;
    document.getElementById('rcount').textContent=realTotal;
    document.getElementById('st-real').textContent=realTotal;
    card.className='radar-card';
    rst.textContent='SCANNING';
    rst.className='rst';
    pill.textContent='last: '+held+'s';
    pill.className='held-pill';
    endOngoing(held,false);

  } else if(d.type==='false'){
    // Motion ended but too brief — false positive
    var held=(d.held_ms/1000).toFixed(1);
    falseTotal++;
    document.getElementById('st-false').textContent=falseTotal;
    card.className='radar-card';
    rst.textContent='SCANNING';
    rst.className='rst';
    pill.textContent='ready';
    pill.className='held-pill';
    endOngoing(held,true);
  }
};

ws.onclose=function(){setTimeout(function(){location.reload();},2000);};
</script>
</body></html>
)rawliteral";

void handleRoot(){ server.send(200,"text/html",INDEX_HTML); }
void webSocketEvent(uint8_t n,WStype_t t,uint8_t*p,size_t l){}

void handleGetLog(){
  File f=SPIFFS.open("/log.txt",FILE_READ);
  if(!f){server.send(200,"text/plain","");return;}
  server.streamFile(f,"text/plain");
  f.close();
}

void handleClearLog(){
  SPIFFS.remove("/log.txt");
  writeLog("=== Log cleared ===");
  server.send(200,"text/plain","ok");
}

void setup(){
  Serial.begin(115200);
  if(!SPIFFS.begin(true)) Serial.println("SPIFFS failed");
  else Serial.println("SPIFFS ready");
  pinMode(PIR_PIN,INPUT);
  WiFi.begin(ssid,password);
  Serial.print("Connecting");
  while(WiFi.status()!=WL_CONNECTED){delay(500);Serial.print(".");}
  Serial.println("\nIP: "+WiFi.localIP().toString());
  server.on("/",handleRoot);
  server.on("/getlog",handleGetLog);
  server.on("/clearlog",handleClearLog);
  server.begin();
  ws.begin();
  ws.onEvent(webSocketEvent);

  bootTime=millis();
  writeLog("=== Started, warming up 30s ===");
  attachInterrupt(digitalPinToInterrupt(PIR_PIN),motionISR,CHANGE);
}

void loop(){
  server.handleClient();
  ws.loop();

  if(!warmedUp){
    if(millis()-bootTime > 30000){
      warmedUp=true;
      motionFlag=false;
      clearFlag=false;
      motionActive=false;
      writeLog("=== Warmup done, now active ===");
      Serial.println("Warmup complete");
    } else {
      motionFlag=false;
      clearFlag=false;
      return;
    }
  }

  // Motion STARTS — fire "start" immediately so dashboard shows ongoing
  if(motionFlag){
    motionFlag=false;
    if(!motionActive){
      motionActive=true;
      motionStartTime=millis();
      Serial.println("Motion START");
      ws.broadcastTXT("{\"type\":\"start\"}");
    }
  }

  // Motion ENDS — calculate total duration, decide real vs false
  if(clearFlag){
    clearFlag=false;
    if(motionActive){
      motionActive=false;
      unsigned long dur=millis()-motionStartTime;
      String timeStr=" | time:"+String(millis()/1000)+"s uptime";

      if(dur < MIN_HOLD_MS){
        totalFalse++;
        String entry="FALSE+ | held:"+String(dur)+"ms"+timeStr;
        writeLog(entry);
        Serial.println(entry);
        ws.broadcastTXT("{\"type\":\"false\",\"held_ms\":"+String(dur)+"}");
      } else {
        totalReal++;
        String entry="MOTION #"+String(totalReal)+" | held:"+String(dur)+"ms"+timeStr;
        writeLog(entry);
        Serial.println(entry);
        ws.broadcastTXT("{\"type\":\"motion\",\"held_ms\":"+String(dur)+"}");
      }
    }
  }
}
