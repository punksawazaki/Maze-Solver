async function apiList() {
    const r = await fetch("/api/mazes");
    return await r.json();
}

function drawRawPreview(canvas, grid) {
    const ctx = canvas.getContext("2d");
    const rows = grid.length;
    const cols = grid[0].length;
    const cell = Math.floor(Math.min(canvas.width/cols, canvas.height/rows));
    ctx.fillStyle = "#111"; ctx.fillRect(0,0,canvas.width,canvas.height);
    for (let r=0;r<rows;r++){
        for (let c=0;c<cols;c++){
            const ch = grid[r][c];
            if (ch === '#') ctx.fillStyle = "#000";
            else if (ch === 'S') ctx.fillStyle = "#0f0";
            else if (ch === 'G') ctx.fillStyle = "#f00";
            else ctx.fillStyle = "#ffffffff";
            ctx.fillRect(c*cell, r*cell, cell, cell);
        }
    }
}

async function loadGallery(){
    const data = await apiList();
    const list = data.mazes || [];
    const grid = document.getElementById("grid");
    grid.innerHTML = "";
    for (const nameWv of list) {
        const name = (typeof nameWv === "string") ? nameWv : (nameWv.s || nameWv);
        const col = document.createElement("div"); col.className = "col-md-4";
        const card = document.createElement("div"); card.className = "maze-card";
        const title = document.createElement("div"); title.textContent = name; title.style.marginBottom="8px";
        card.appendChild(title);
        const cv = document.createElement("canvas"); cv.width=300; cv.height=200; card.appendChild(cv);
        const btnRow = document.createElement("div"); btnRow.className="mt-2";
        const openBtn = document.createElement("a"); openBtn.className="btn btn-sm btn-primary me-2"; openBtn.textContent="Start"; openBtn.href=`/run/${encodeURIComponent(name)}`;
        const editBtn = document.createElement("a"); editBtn.className="btn btn-sm btn-outline-light me-2"; editBtn.textContent="Edit"; editBtn.href=`/create?load=${encodeURIComponent(name)}`;
        const delBtn = document.createElement("button"); delBtn.className="btn btn-sm btn-danger"; delBtn.textContent="Delete";
        delBtn.onclick = async ()=>{ if (!confirm("Delete?")) return; await fetch(`/api/maze/delete/${encodeURIComponent(name)}`, { method: "POST" }); loadGallery(); };
        btnRow.appendChild(openBtn); btnRow.appendChild(editBtn); btnRow.appendChild(delBtn);
        card.appendChild(btnRow);
        col.appendChild(card);
        grid.appendChild(col);

        // fetch raw maze and draw preview
        (async function(canvasEl, nm) {
            const resp = await fetch(`/api/maze/${encodeURIComponent(nm)}`);
            if (!resp.ok) return;
            const data = await resp.json();
            const rows = data.contents.split(/\r?\n/).filter(l=>l.length);
            const g = rows.map(r=>r.split(''));
            drawRawPreview(canvasEl, g);
        })(cv, name);
    }
}

document.addEventListener("DOMContentLoaded", () => {
    loadGallery();
    document.getElementById("uploadBtn").addEventListener("click", async () => {
        const filename = prompt("filename (e.g. mymaze.txt)"); if (!filename) return;
        const contents = prompt("paste maze text lines (# . S G)"); if (contents==null) return;
        await fetch("/api/maze/upload", { method: "POST", body: JSON.stringify({ filename, contents }) });
        loadGallery();
    });
});