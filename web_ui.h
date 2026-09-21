#ifndef WEB_UI_H
#define WEB_UI_H

#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover">
<meta name="theme-color" content="#0f172a">
<meta name="apple-mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
<title>Tankpreis</title>
<style>
:root{
--bg:#0f172a;
--card:#1e293b;
--border:#334155;
--text:#f8fafc;
--muted:#94a3b8;
--primary:#38bdf8;
--green:#22c55e;
--red:#ef4444;
--orange:#f59e0b;
--radius:12px;
}
*{box-sizing:border-box;margin:0;padding:0;-webkit-tap-highlight-color:transparent}
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;background:var(--bg);color:var(--text);min-height:100vh;padding:calc(10px + env(safe-area-inset-top,0px)) 12px calc(30px + env(safe-area-inset-bottom,0px))}
.c{max-width:500px;margin:0 auto}
h1{text-align:center;padding:12px 0 16px;font-size:1.45em;font-weight:700;color:var(--primary);letter-spacing:-0.02em}
.tabs{display:flex;gap:6px;background:var(--card);padding:5px;border-radius:var(--radius);margin-bottom:16px;border:1px solid var(--border);overflow-x:auto;-webkit-overflow-scrolling:touch;scrollbar-width:none}
.tabs::-webkit-scrollbar{display:none}
.tab{flex:1;min-width:fit-content;text-align:center;padding:10px 12px;border-radius:8px;font-size:.88em;font-weight:600;color:var(--muted);cursor:pointer;white-space:nowrap;transition:.2s;user-select:none}
.tab.a{background:var(--primary);color:#0f172a;box-shadow:0 2px 8px rgba(56,189,248,.35)}
.pg{display:none}.pg.a{display:block}
input[type=text],input[type=password],input[type=number],select{width:100%;min-height:46px;padding:10px 14px;margin:6px 0;border:1px solid var(--border);border-radius:8px;background:#0f172a;color:var(--text);font-size:16px}
input:focus,select:focus{border-color:var(--primary);box-shadow:0 0 0 3px rgba(56,189,248,.2);outline:none}
.btn{display:inline-flex;align-items:center;justify-content:center;min-height:46px;padding:10px 18px;margin:6px 0;width:100%;border:none;border-radius:8px;background:var(--primary);color:#0f172a;font-weight:700;font-size:.95em;cursor:pointer;transition:.15s;touch-action:manipulation}
.btn:active{transform:scale(.98);opacity:.9}
.bd{background:var(--red);color:#fff}
.bg{background:var(--green);color:#fff}
.bs{min-height:38px;width:auto;padding:6px 14px;font-size:.85em;margin:2px}
.card{background:var(--card);border-radius:var(--radius);padding:14px 16px;margin:10px 0;border:1px solid var(--border);box-shadow:0 4px 12px rgba(0,0,0,.15)}
.card.ac{border-color:var(--primary);box-shadow:0 0 16px rgba(56,189,248,.25)}
.fx{display:flex;gap:10px;align-items:center}
.fb{display:flex;justify-content:space-between;align-items:center;gap:8px}
.lb{font-size:.85em;font-weight:600;color:var(--muted);margin-top:8px;margin-bottom:2px}
.st{padding:12px 14px;border-radius:8px;margin:8px 0;font-size:.9em;line-height:1.4}
.so{background:rgba(34,197,94,.15);border:1px solid rgba(34,197,94,.3);color:#4ade80}
.se{background:rgba(239,68,68,.15);border:1px solid rgba(239,68,68,.3);color:#f87171}
.si{background:rgba(56,189,248,.15);border:1px solid rgba(56,189,248,.3);color:var(--primary)}
.wi{padding:12px 8px;border-bottom:1px solid var(--border);cursor:pointer;border-radius:6px;transition:.15s}
.wi:hover,.wi:active{background:#25334d}
.sig{color:var(--muted);font-size:.8em}
.pb{font-size:2.8em;text-align:center;color:var(--primary);font-weight:800;padding:12px 0;letter-spacing:-0.03em}
.rc{display:flex;align-items:center;gap:12px}
.rc input[type=range]{flex:1;height:32px;accent-color:var(--primary);cursor:pointer}
.sp{display:inline-block;width:18px;height:18px;border:3px solid rgba(255,255,255,.2);border-top-color:var(--primary);border-radius:50%;animation:spin 1s linear infinite;vertical-align:middle;margin-right:6px}
@keyframes spin{to{transform:rotate(360deg)}}
.h{display:none !important}
.fb0{display:inline-block;padding:3px 8px;border-radius:6px;font-size:.78em;font-weight:700}
.fe5{background:var(--green);color:#000}
.fe10{background:var(--orange);color:#000}
.fdiesel{background:var(--primary);color:#000}
.sn{font-weight:700;font-size:.95em}.sa{font-size:.82em;color:var(--muted);margin-top:3px}
.mt{margin-top:10px}.mb{margin-bottom:10px}
a{color:var(--primary);text-decoration:none}
a:hover{text-decoration:underline}
.stb{display:inline-block;padding:8px 16px;margin:3px;border:1px solid var(--border);border-radius:8px;cursor:pointer;font-size:.88em;font-weight:600;transition:.15s;background:#0f172a;color:var(--muted)}
.stb.a{background:var(--primary);color:#0f172a;border-color:var(--primary)}
.mk{font-family:monospace;font-size:1.1em;color:var(--primary);letter-spacing:1px}
</style>
</head>
<body>
<div class="c">
<h1>&#9981; Tankpreis</h1>
<div class="tabs" id="tabs">
<div class="tab a" data-p="wifi">&#128225; WiFi</div>
<div class="tab" data-p="api">&#128273; API</div>
<div class="tab" data-p="search">&#128270; Suche</div>
<div class="tab" data-p="favs">&#11088; Favoriten</div>
<div class="tab" data-p="set">&#9881; Setup</div>
</div>

<!-- ===== WiFi Page ===== -->
<div id="p-wifi" class="pg a">
<div id="ws"></div>
<div id="wmsg"></div>

<div class="card">
<p class="lb" style="margin-top:0">&#128246; Gespeicherte Netzwerke</p>
<div id="wsl"></div>
</div>

<div class="card">
<p class="lb" style="margin-top:0">&#10133; Netzwerk hinzufuegen / Manuell</p>
<input type="text" id="wms" placeholder="SSID (Netzwerkname)" autocapitalize="none" autocorrect="off">
<input type="password" id="wmp" placeholder="WLAN-Passwort">
<div class="fx" style="margin-top:6px">
<button class="btn" style="flex:1" onclick="addWifi(false)">&#128190; Speichern</button>
<button class="btn bg" style="flex:1" onclick="addWifi(true)">&#9654; Verbinden</button>
</div>
</div>

<button class="btn" onclick="scanW()">&#128270; Netzwerke scannen</button>
<div id="wl"></div>
</div>

<!-- ===== API Page ===== -->
<div id="p-api" class="pg">
<div class="card">
<p class="lb" style="margin-top:0">Tankerkoenig API-Key</p>
<div id="akd" class="h" style="padding:8px 0">
<span class="mk" id="akm"></span>
<span class="lb"> &#9989; gespeichert</span>
</div>
<input type="text" id="ak" placeholder="API-Key eingeben..." autocapitalize="none" autocorrect="off">
<p style="font-size:.78em;color:var(--muted);margin-top:6px">
Kostenloser Key unter: <a href="https://creativecommons.tankerkoenig.de" target="_blank">creativecommons.tankerkoenig.de</a>
</p>
<button class="btn mt" onclick="saveK()">&#128190; Speichern</button>
</div>
<div id="as"></div>
</div>

<!-- ===== Search Page ===== -->
<div id="p-search" class="pg">
<div class="mb" style="display:flex;gap:6px;overflow-x:auto">
<span class="stb a" id="st1" onclick="showSM(1)">&#128238; PLZ</span>
<span class="stb" id="st2" onclick="showSM(2)">&#128205; Koordinaten</span>
<span class="stb" id="st3" onclick="showSM(3)">&#128273; Station-ID</span>
</div>

<div id="sm1" class="card">
<div class="fx">
<div style="flex:2"><p class="lb" style="margin-top:0">Postleitzahl</p><input type="text" id="splz" placeholder="z.B. 80331" maxlength="5" pattern="[0-9]*" inputmode="numeric"></div>
<div style="flex:1"><p class="lb" style="margin-top:0">Radius km</p><input type="number" id="spr" value="5" min="1" max="25"></div>
</div>
<button class="btn mt" onclick="srcP()">&#128270; Tankstellen suchen</button>
</div>

<div id="sm2" class="card h">
<div class="fx">
<div style="flex:1"><p class="lb" style="margin-top:0">Breitengrad (Lat)</p><input type="text" id="sl" placeholder="z.B. 53.856" inputmode="decimal"></div>
<div style="flex:1"><p class="lb" style="margin-top:0">Laengengrad (Lng)</p><input type="text" id="slo" placeholder="z.B. 10.657" inputmode="decimal"></div>
</div>
<div class="fx mt">
<div style="flex:1"><p class="lb">Radius (km)</p><input type="number" id="sr" value="5" min="1" max="25"></div>
<div style="flex:1;display:flex;align-items:flex-end">
<button type="button" class="btn bs" style="width:100%" onclick="getGPS()">&#128205; Mein Standort</button>
</div>
</div>
<button class="btn mt" onclick="srcS()">&#128270; Tankstellen suchen</button>
</div>

<div id="sm3" class="card h">
<p class="lb" style="margin-top:0">Tankstellen-ID (UUID)</p>
<input type="text" id="sid" placeholder="z.B. 474e5046-deaf-4f9b-9a32-9797b778f047" autocapitalize="none" autocorrect="off">
<p class="lb">Kraftstoff</p>
<select id="sft">
<option value="e5">Super E5</option>
<option value="e10">Super E10</option>
<option value="diesel">Diesel</option>
</select>
<button class="btn bg mt" onclick="addMF()">&#11088; Als Favorit hinzufuegen</button>
<div id="mfs" class="mt"></div>
</div>

<div id="ss"></div>
<div id="sres"></div>
</div>

<!-- ===== Favorites Page ===== -->
<div id="p-favs" class="pg">
<div id="cp"></div>
<div id="fl"></div>
<div id="fe" class="card" style="text-align:center;color:var(--muted)">
Keine Favoriten. Suche Tankstellen und fuege sie hinzu!
</div>
</div>

<!-- ===== Settings Page ===== -->
<div id="p-set" class="pg">
<div class="card">
<p class="lb" style="margin-top:0">&#9200; Anzeige-Wechsel (Sekunden)</p>
<div class="fx">
<div style="flex:1">
<p class="lb">Uhrzeit: <b id="cv">20</b>s</p>
<input type="range" id="scd" min="5" max="60" value="20" oninput="D('cv').textContent=this.value">
</div>
<div style="flex:1">
<p class="lb">Preis: <b id="pv">10</b>s</p>
<input type="range" id="spd" min="5" max="60" value="10" oninput="D('pv').textContent=this.value">
</div>
</div>
</div>

<div class="card">
<div class="fb">
<span style="font-weight:700">&#127769; Nachtmodus</span>
<label style="display:flex;align-items:center;gap:8px;cursor:pointer;font-size:.95em">
<input type="checkbox" id="snm" onchange="toggleNM()" style="width:18px;height:18px"> Aktivieren
</label>
</div>
<div id="nmb" class="h mt">
<div class="fx">
<div style="flex:1"><p class="lb">Von (HH:MM)</p><input type="text" id="sns" placeholder="23:00" maxlength="5"></div>
<div style="flex:1"><p class="lb">Bis (HH:MM)</p><input type="text" id="sne" placeholder="07:00" maxlength="5"></div>
</div>
<p class="lb">Nacht-Helligkeit (0 = Aus)</p>
<div class="rc">
<input type="range" id="snb" min="0" max="7" value="1" oninput="D('nbv').textContent=this.value==0?'Aus':this.value">
<span id="nbv" style="min-width:30px;font-weight:700">1</span>
</div>
</div>
</div>

<div class="card">
<p class="lb" style="margin-top:0">&#9881; Aktualisierungsintervall (Tankerkoenig)</p>
<div class="rc">
<input type="range" id="si" min="5" max="60" value="10" oninput="D('iv').textContent=this.value">
<span id="iv" style="min-width:45px;font-weight:700">10 min</span>
</div>
</div>

<div class="card">
<p class="lb" style="margin-top:0">&#9728; Display-Helligkeit (Tag)</p>
<div class="rc">
<input type="range" id="sb" min="0" max="7" value="5" oninput="D('bv').textContent=this.value">
<span id="bv" style="min-width:30px;font-weight:700">5</span>
</div>
</div>

<div class="card">
<p class="lb" style="margin-top:0">&#9981; Preis-Format auf Display</p>
<select id="spf">
<option value="0">4-stellig exakt (z.B. 2379)</option>
<option value="1">Gerundet mit Unterstrich (z.B. 2_38)</option>
<option value="2">Gerundet mit Mittelstrich (z.B. 2-38)</option>
<option value="3">Gerundet ohne Strich (z.B. 2 38)</option>
</select>
</div>

<button class="btn mt" onclick="saveS()">&#128190; Einstellungen speichern</button>
<div id="sts" class="mt"></div>

<div class="card mt">
<p class="lb" style="margin-top:0">&#9881; System</p>
<div class="fx">
<button class="btn" style="flex:1" onclick="sysReboot()">&#128260; Neustart</button>
<button class="btn bd" style="flex:1" onclick="sysReset()">&#9888; Reset</button>
</div>
</div>
</div>
</div>

<script>
function D(i){return document.getElementById(i)}
document.getElementById('tabs').addEventListener('click',function(e){
var t=e.target;if(!t.classList.contains('tab'))return;
document.querySelectorAll('.pg').forEach(function(p){p.classList.remove('a')});
document.querySelectorAll('.tab').forEach(function(x){x.classList.remove('a')});
D('p-'+t.getAttribute('data-p')).classList.add('a');
t.classList.add('a');
var p=t.getAttribute('data-p');
if(p==='wifi')loadWS();
if(p==='favs')loadF();
if(p==='set')loadSt();
if(p==='api')loadK();
});
function api(m,u,d){
var o={method:m,headers:{'Content-Type':'application/json'}};
if(d)o.body=JSON.stringify(d);
return fetch(u,o).then(function(r){return r.json()});
}

/* ===== WiFi ===== */
function loadWS(){
api('GET','/api/status').then(function(d){
var s=D('ws');
if(d.wifi_connected)s.innerHTML='<div class="st so">&#9989; Verbunden mit <b>'+d.wifi_ssid+'</b><br>IP: '+d.wifi_ip+'</div>';
else s.innerHTML='<div class="st se">&#10060; Nicht verbunden (AP: Tankpreis-Setup)</div>';
}).catch(function(){});
loadSavedWifi();
}

function loadSavedWifi(){
api('GET','/api/wifi/saved').then(function(list){
var el=D('wsl');
if(!list||list.length===0){
el.innerHTML='<div style="color:var(--muted);font-size:0.85em;padding:4px 0">Keine Netzwerke gespeichert</div>';
return;
}
var h='';
list.forEach(function(item){
var esc=item.ssid.replace(/\\/g,'\\\\').replace(/'/g,"\\'");
h+='<div class="fb" style="padding:10px 0;border-bottom:1px solid var(--border)">';
h+='<span style="font-weight:600">&#128246; '+item.ssid+'</span>';
h+='<div style="display:flex;gap:6px">';
h+='<button class="btn bs" onclick="connectSaved(\''+esc+'\')">&#9654;</button>';
h+='<button class="btn bs bd" onclick="removeWifi('+item.index+')">&#128465;</button>';
h+='</div></div>';
});
el.innerHTML=h;
}).catch(function(){});
}

function scanW(){
D('wl').innerHTML='<div class="sp"></div> Scanne Netzwerke...';
pollScan(0);
}

function pollScan(attempts){
if(attempts>15){
D('wl').innerHTML='<div class="st se">&#10060; Zeitueberschreitung beim Scannen</div>';
return;
}
api('GET','/api/wifi/scan').then(function(d){
if(d&&d.status==='scanning'){
setTimeout(function(){pollScan(attempts+1)},1000);
return;
}
if(!Array.isArray(d)){
D('wl').innerHTML='<div class="st se">&#10060; Fehler beim Scannen</div>';
return;
}
if(d.length===0){
D('wl').innerHTML='<p style="padding:10px;color:var(--muted)">Keine Netzwerke gefunden</p>';
return;
}
var h='';
d.forEach(function(n){
var lk=n.enc?'&#128274;':'';
var sg=n.rssi>-50?'&#9608;&#9606;&#9604;&#9602;':n.rssi>-70?'_&#9606;&#9604;&#9602;':'__&#9604;&#9602;';
var esc=n.ssid.replace(/\\/g,'\\\\').replace(/'/g,"\\'");
h+='<div class="wi" onclick="selW(\''+esc+'\')">'
+'<div class="fb"><span>'+lk+' <b>'+n.ssid+'</b></span><span class="sig">'+sg+' '+n.rssi+'dBm</span></div></div>';
});
D('wl').innerHTML=h;
}).catch(function(){
D('wl').innerHTML='<div class="st se">&#10060; Netzwerkfehler beim Scannen</div>';
});
}

function selW(s){
D('wms').value=s;
D('wmp').value='';
D('wmp').focus();
}

function addWifi(connectNow){
var s=D('wms').value.trim(),p=D('wmp').value;
if(!s){D('wmsg').innerHTML='<div class="st se">Bitte SSID eingeben</div>';return}
if(connectNow) D('wmsg').innerHTML='<div class="st si"><div class="sp"></div> Starte Verbindung mit '+s+'...</div>';
else D('wmsg').innerHTML='<div class="st si"><div class="sp"></div> Speichere '+s+'...</div>';
api('POST','/api/wifi/add',{ssid:s,password:p,connect_now:connectNow}).then(function(d){
if(d.success){
D('wms').value='';D('wmp').value='';
loadSavedWifi();
if(connectNow){
D('wmsg').innerHTML='<div class="st so">&#9989; Verbindung wird hergestellt!<br>Achte auf das Display (IP-Adresse wird angezeigt).</div>';
}else{
D('wmsg').innerHTML='<div class="st so">&#9989; Netzwerk gespeichert!</div>';
setTimeout(function(){D('wmsg').innerHTML=''},4000);
}
}else{
D('wmsg').innerHTML='<div class="st se">&#10060; '+(d.message||'Fehler')+'</div>';
}
}).catch(function(){D('wmsg').innerHTML='<div class="st se">&#10060; Verbindungsfehler zum Geraet</div>'});
}

function connectSaved(ssid){
D('wmsg').innerHTML='<div class="st si"><div class="sp"></div> Starte Verbindung mit '+ssid+'...</div>';
api('POST','/api/wifi/add',{ssid:ssid,password:'',connect_now:true}).then(function(d){
if(d.success){
D('wmsg').innerHTML='<div class="st so">&#9989; Verbindung zu <b>'+ssid+'</b> wird hergestellt!<br>Achte auf das Display (IP-Adresse wird angezeigt).</div>';
}else{
D('wmsg').innerHTML='<div class="st se">&#10060; Fehler</div>';
}
}).catch(function(){D('wmsg').innerHTML='<div class="st se">&#10060; Verbindungsfehler</div>'});
}

function removeWifi(idx){
if(!confirm('Netzwerk loeschen?'))return;
api('DELETE','/api/wifi/remove?index='+idx).then(function(d){
if(d.success){
loadSavedWifi();
D('wmsg').innerHTML='<div class="st si">Netzwerk geloescht</div>';
setTimeout(function(){D('wmsg').innerHTML=''},3000);
}else alert(d.message||'Fehler');
});
}

/* ===== API Key ===== */
var gApiKey='';
function getApiKey(){return gApiKey||(D('ak')?D('ak').value.trim():'')}
function directSearch(la,ln,ra){
var k=getApiKey();
if(!k)return Promise.reject(new Error('Kein API-Key vorhanden'));
return fetch('https://creativecommons.tankerkoenig.de/json/list.php?lat='+la+'&lng='+ln+'&rad='+ra+'&sort=dist&type=all&apikey='+k).then(function(r){return r.json()});
}
function loadK(){
api('GET','/api/config').then(function(d){
if(d.api_key)gApiKey=d.api_key;
if(d.api_key_set){
D('akd').classList.remove('h');
D('akm').textContent=d.api_key_masked;
D('ak').placeholder='Neuen Key eingeben zum Aendern...';
}else{
D('akd').classList.add('h');
D('ak').placeholder='API-Key eingeben...';
}
D('ak').value='';
});
}
function saveK(){
var k=D('ak').value.trim();
if(!k){D('as').innerHTML='<div class="st se">Bitte Key eingeben</div>';return}
api('POST','/api/config',{api_key:k}).then(function(d){
if(d.success){D('as').innerHTML='<div class="st so">&#9989; API-Key gespeichert!</div>';loadK()}
else D('as').innerHTML='<div class="st se">&#10060; Fehler</div>';
setTimeout(function(){D('as').innerHTML=''},3000);
});
}

/* ===== Search Mode Switching ===== */
function showSM(n){
for(var i=1;i<=3;i++){
D('sm'+i).classList.toggle('h',i!==n);
D('st'+i).classList.toggle('a',i===n);
}
D('ss').innerHTML='';D('sres').innerHTML='';
}

/* ===== GPS / IP Geolocation ===== */
function getGPS(){
D('ss').innerHTML='<div class="sp"></div> Standort wird ermittelt...';
function tryIP(){
fetch('https://ipwho.is/')
.then(function(r){return r.json()})
.then(function(d){
if(d&&d.latitude&&d.longitude){
D('sl').value=Number(d.latitude).toFixed(4);
D('slo').value=Number(d.longitude).toFixed(4);
var city=d.city||d.postal||'';
D('ss').innerHTML='<div class="st so">&#9989; Standort via IP ermittelt'+(city?' ('+city+')':'')+'</div>';
setTimeout(function(){D('ss').innerHTML=''},3500);
}else{
throw new Error('IP ungueltig');
}
})
.catch(function(){
D('ss').innerHTML='<div class="st se">&#10060; GPS vom Browser ohne HTTPS blockiert.<br>Tipp: Bitte nutze oben den Tab <b>&#128238; PLZ</b> (z.B. 23558)!</div>';
});
}
if(window.isSecureContext&&navigator.geolocation){
navigator.geolocation.getCurrentPosition(function(pos){
D('sl').value=pos.coords.latitude.toFixed(4);
D('slo').value=pos.coords.longitude.toFixed(4);
D('ss').innerHTML='<div class="st so">&#9989; GPS-Standort ermittelt</div>';
setTimeout(function(){D('ss').innerHTML=''},2500);
},function(){
tryIP();
},{enableHighAccuracy:true,timeout:6000});
}else{
tryIP();
}
}

/* ===== PLZ Search ===== */
function srcP(){
var plz=D('splz').value.trim(),rad=D('spr').value;
if(!plz||plz.length!==5){D('ss').innerHTML='<div class="st se">Bitte gueltige PLZ eingeben (5 Ziffern)</div>';return}
D('sres').innerHTML='<div class="sp"></div> Suche Tankstellen bei PLZ '+plz+'...';D('ss').innerHTML='';
api('GET','/api/stations/searchplz?plz='+plz+'&rad='+rad).then(function(d){
if(d&&d.ok&&d.stations&&d.stations.length>0){
renderStations(d);
}else{
throw new Error((d&&d.message)||'ESP Suche fehlgeschlagen');
}
}).catch(function(e){
fetch('https://photon.komoot.io/api/?q='+plz+'+Germany&limit=1')
.then(function(r){return r.json()})
.then(function(geo){
if(geo.features&&geo.features.length>0){
var c=geo.features[0].geometry.coordinates;
return directSearch(c[1],c[0],rad);
}else{
throw new Error('PLZ nicht gefunden');
}
})
.then(function(d2){renderStations(d2)})
.catch(function(err){D('sres').innerHTML='<div class="st se">Fehler: '+(err.message||e)+'</div>'});
});
}

/* ===== Coordinate Search ===== */
function srcS(){
var la=D('sl').value.trim(),ln=D('slo').value.trim(),ra=D('sr').value;
if(!la&&!ln){D('ss').innerHTML='<div class="st se">Bitte Koordinaten eingeben</div>';return}
if(la.indexOf(',')!==-1&&!ln&&la.split(',').length===2){
var parts=la.split(',');
la=parts[0].trim();ln=parts[1].trim();
D('sl').value=la;D('slo').value=ln;
}
la=la.replace(',','.');ln=ln.replace(',','.');
var fLat=parseFloat(la),fLng=parseFloat(ln);
if(isNaN(fLat)||isNaN(fLng)){D('ss').innerHTML='<div class="st se">Ungueltige Koordinaten</div>';return}
if(fLat<20&&fLng>45){var tmp=fLat;fLat=fLng;fLng=tmp;D('sl').value=fLat;D('slo').value=fLng;}
D('sres').innerHTML='<div class="sp"></div> Suche...';D('ss').innerHTML='';
api('GET','/api/stations/search?lat='+fLat+'&lng='+fLng+'&rad='+ra).then(function(d){
if(d&&d.ok&&d.stations&&d.stations.length>0){
renderStations(d);
}else{
throw new Error((d&&d.message)||'Fehler');
}
}).catch(function(e){
directSearch(fLat,fLng,ra).then(function(d2){
renderStations(d2);
}).catch(function(err){
D('sres').innerHTML='<div class="st se">Fehler: '+(err.message||e)+'</div>';
});
});
}

/* ===== Shared station results renderer ===== */
function renderStations(d){
if(!d.ok){D('sres').innerHTML='<div class="st se">'+(d.message||'API Fehler')+'</div>';return}
var h='';
(d.stations||[]).forEach(function(s){
var pr='';
if(s.e5)pr+='<span class="fb0 fe5">E5: '+s.e5.toFixed(3)+'&euro;</span> ';
if(s.e10)pr+='<span class="fb0 fe10">E10: '+s.e10.toFixed(3)+'&euro;</span> ';
if(s.diesel)pr+='<span class="fb0 fdiesel">Diesel: '+s.diesel.toFixed(3)+'&euro;</span>';
var st=s.isOpen?'&#128994; Offen':'&#128308; Geschlossen';
var nm=((s.brand||'')+' '+(s.name||'')).trim();
h+='<div class="card"><div class="fb"><span class="sn">'+nm+'</span><span style="font-size:.75em">'+s.dist+'km</span></div>';
h+='<div class="sa">'+(s.street||'')+' '+(s.houseNumber||'')+', '+(s.postCode||'')+' '+(s.place||'')+'</div>';
h+='<div class="mt" style="font-size:.8em">'+st+'</div>';
h+='<div class="mt">'+pr+'</div>';
h+='<div class="fx mt">';
var en=nm.replace(/'/g,"\\'").replace(/"/g,'');
if(s.e5)h+='<button class="btn bs bg" onclick="addF(\''+s.id+"','"+en+"','e5')\">&#11088; E5</button>";
if(s.e10)h+='<button class="btn bs bg" onclick="addF(\''+s.id+"','"+en+"','e10')\">&#11088; E10</button>";
if(s.diesel)h+='<button class="btn bs bg" onclick="addF(\''+s.id+"','"+en+"','diesel')\">&#11088; Diesel</button>";
h+='</div></div>';
});
D('sres').innerHTML=h||'<p style="padding:10px">Keine Tankstellen gefunden</p>';
}

function addF(id,nm,fu){
api('POST','/api/favorites',{id:id,name:nm,fuel:fu}).then(function(d){
if(d.success)alert('Favorit hinzugefuegt!');
else alert(d.message||'Fehler');
});
}

/* ===== Manual Station ID ===== */
function addMF(){
var id=D('sid').value.trim(),fuel=D('sft').value;
if(!id){D('mfs').innerHTML='<div class="st se">Bitte Station-ID eingeben</div>';return}
D('mfs').innerHTML='<div class="sp"></div> Lade Stationsinfo...';
api('GET','/api/stations/detail?id='+id).then(function(d){
if(!d.ok){D('mfs').innerHTML='<div class="st se">'+(d.message||'Station nicht gefunden')+'</div>';return}
var s=d.station;
var nm=((s.brand||'')+' '+(s.name||'')).trim()||id;
api('POST','/api/favorites',{id:s.id,name:nm,fuel:fuel}).then(function(r){
if(r.success)D('mfs').innerHTML='<div class="st so">&#9989; '+nm+' ('+fuel.toUpperCase()+') hinzugefuegt!</div>';
else D('mfs').innerHTML='<div class="st se">'+(r.message||'Fehler')+'</div>';
});
}).catch(function(e){D('mfs').innerHTML='<div class="st se">Fehler: '+e+'</div>'});
}

/* ===== Favorites ===== */
function loadF(){
api('GET','/api/favorites').then(function(d){
var l=d.favorites||[];
if(l.length===0){D('fl').innerHTML='';D('fe').style.display='block';D('cp').innerHTML='';return}
D('fe').style.display='none';

var cpHtml='';
var activeFavs=l.filter(function(f){return f.active});
if(activeFavs.length>0){
cpHtml+='<div class="card ac"><div class="fb" style="margin-bottom:6px"><b style="font-size:.9em">&#9981; Aktive Anzeige (Wechsel):</b><span style="font-size:.8em;color:var(--muted)">'+activeFavs.length+' gewaehlt</span></div><div class="fx" style="flex-wrap:wrap;gap:8px">';
activeFavs.forEach(function(f){
var pStr=(f.price&&f.price>0)?(f.price.toFixed(3)+' &euro;'):'--.--- &euro;';
cpHtml+='<div style="flex:1;min-width:120px;background:rgba(255,255,255,0.06);padding:8px;border-radius:6px;text-align:center">';
cpHtml+='<div style="font-size:1.2em;font-weight:700;color:var(--accent)">'+pStr+'</div>';
cpHtml+='<div style="font-size:.8em;white-space:nowrap;overflow:hidden;text-overflow:ellipsis">'+f.name+'</div>';
cpHtml+='<span class="fb0 f'+f.fuel+'" style="margin-top:4px">'+f.fuel.toUpperCase()+'</span>';
cpHtml+='</div>';
});
cpHtml+='</div></div>';
}
D('cp').innerHTML=cpHtml;

var h='';
l.forEach(function(f,i){
var ia=!!f.active;
var pInfo=(f.price&&f.price>0)?(' <b style="color:var(--accent)">('+f.price.toFixed(3)+' &euro;)</b>'):'';
h+='<div class="card'+(ia?' ac':'')+'"><div class="fb"><div><span class="sn">'+f.name+'</span> '
+'<span class="fb0 f'+f.fuel+'">'+f.fuel.toUpperCase()+'</span>'+pInfo+'</div></div>';
h+='<div class="fx mt">';
if(ia){
h+='<button class="btn bs bg" style="flex:2" onclick="togF('+i+')">&#9989; Aktiv (im Wechsel)</button>';
}else{
h+='<button class="btn bs" style="flex:2" onclick="togF('+i+')">&#9654; Aktivieren</button>';
}
h+='<button class="btn bs bd" onclick="delF('+i+')">&#128465;</button></div></div>';
});
D('fl').innerHTML=h;
});
}
function togF(i){api('POST','/api/favorites/toggle?index='+i).then(function(d){if(d.success)loadF()})}
function actF(i){api('POST','/api/favorites/activate?index='+i).then(function(d){if(d.success)loadF()})}
function delF(i){
if(!confirm('Favorit loeschen?'))return;
api('DELETE','/api/favorites?index='+i).then(function(d){if(d.success)loadF()});
}

/* ===== Settings ===== */
function toggleNM(){
D('nmb').classList.toggle('h',!D('snm').checked);
}
function pad2(n){return (n<10?'0':'')+n}

function loadSt(){
api('GET','/api/config').then(function(d){
var iv=Math.round((d.interval||600)/60);
D('si').value=iv;D('iv').textContent=iv;
var br=d.brightness!==undefined?d.brightness:5;
D('sb').value=br;D('bv').textContent=br;

var cd=d.clock_duration||20;
D('scd').value=cd;D('cv').textContent=cd;
var pd=d.price_duration||10;
D('spd').value=pd;D('pv').textContent=pd;

var pf=d.price_format!==undefined?d.price_format:0;
D('spf').value=pf;

var nm=!!d.night_mode;
D('snm').checked=nm;
D('nmb').classList.toggle('h',!nm);
var sh=d.night_start_hour!==undefined?d.night_start_hour:23;
var sm=d.night_start_min!==undefined?d.night_start_min:0;
var eh=d.night_end_hour!==undefined?d.night_end_hour:7;
var em=d.night_end_min!==undefined?d.night_end_min:0;
D('sns').value=pad2(sh)+':'+pad2(sm);
D('sne').value=pad2(eh)+':'+pad2(em);

var nb=d.night_brightness!==undefined?d.night_brightness:1;
D('snb').value=nb;
D('nbv').textContent=nb==0?'Aus':nb;
});
}

function saveS(){
var iv=parseInt(D('si').value)*60,br=parseInt(D('sb').value);
var cd=parseInt(D('scd').value),pd=parseInt(D('spd').value);
var pf=parseInt(D('spf').value)||0;
var nm=D('snm').checked;
var sParts=(D('sns').value||'23:00').split(':');
var eParts=(D('sne').value||'07:00').split(':');
var sh=parseInt(sParts[0])||0,sm=parseInt(sParts[1])||0;
var eh=parseInt(eParts[0])||0,em=parseInt(eParts[1])||0;
var nb=parseInt(D('snb').value);

api('POST','/api/config',{
interval:iv,
brightness:br,
clock_duration:cd,
price_duration:pd,
price_format:pf,
night_mode:nm,
night_start_hour:sh,
night_start_min:sm,
night_end_hour:eh,
night_end_min:em,
night_brightness:nb
}).then(function(d){
D('sts').innerHTML=d.success?'<div class="st so">&#9989; Gespeichert!</div>':'<div class="st se">&#10060; Fehler</div>';
setTimeout(function(){D('sts').innerHTML=''},3000);
});
}

/* ===== System ===== */
function sysReboot(){
if(!confirm('Wemos D1 Mini neu starten?'))return;
D('sts').innerHTML='<div class="st si"><div class="sp"></div> Neustart laeuft... Bitte Seite in ca. 10s neu laden.</div>';
api('POST','/api/system/reboot').then(function(){
setTimeout(function(){location.reload()},8000);
});
}

function sysReset(){
if(!confirm('ACHTUNG: Wirklich alle Einstellungen und WiFi-Zugangsdaten auf Werkseinstellung zuruecksetzen?'))return;
D('sts').innerHTML='<div class="st se"><div class="sp"></div> Speicher wird formatiert... Neu verbinden mit AP Tankpreis-Config!</div>';
api('POST','/api/system/reset');
}

/* ===== Init ===== */
loadWS();
loadK();
</script>
</body>
</html>)rawliteral";

#endif // WEB_UI_H
