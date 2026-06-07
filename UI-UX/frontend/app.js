
// SmartCare global interactions
window.addEventListener('load',()=>setTimeout(()=>document.querySelector('.loader')?.classList.add('hide'),450));
document.body.insertAdjacentHTML('afterbegin','<div class="loader"><div><div class="loader-ring"></div><div class="loader-text">SmartCare</div></div></div><div class="nav-overlay" id="navOverlay"></div><span class="bg-orb orb-1"></span><span class="bg-orb orb-2"></span>');
document.querySelectorAll('.nav-links a').forEach(a=>{if(a.dataset.page===document.body.dataset.page)a.classList.add('active')});
const hamb=document.getElementById('hamb'), nav=document.getElementById('navLinks'), overlay=document.getElementById('navOverlay');
function closeNav(){nav?.classList.remove('open');overlay?.classList.remove('show')}
hamb?.addEventListener('click',()=>{nav?.classList.toggle('open');overlay?.classList.toggle('show')});overlay?.addEventListener('click',closeNav);document.addEventListener('keydown',e=>{if(e.key==='Escape')closeNav()});
document.querySelectorAll('.nav-links a').forEach(a=>a.addEventListener('click',closeNav));
document.querySelectorAll('.btn').forEach(btn=>btn.classList.add('ripple'));
// auth behavior
const authForms=document.querySelectorAll('form[data-auth]');authForms.forEach(f=>f.addEventListener('submit',e=>{e.preventDefault();const type=f.dataset.auth;if(type==='register'){const p=f.querySelector('#password'),c=f.querySelector('#confirmPassword');if(p&&c&&p.value!==c.value){alert('Password dan Confirm Password belum sama.');return}if(p&&p.value.length<6){alert('Password minimal 6 karakter.');return}}location.href='index.html'}));
document.querySelectorAll('.show-pass').forEach(btn=>btn.addEventListener('click',()=>{const target=document.getElementById(btn.dataset.target);if(!target)return;target.type=target.type==='password'?'text':'password';btn.textContent=target.type==='password'?'Show password':'Hide password'}));
