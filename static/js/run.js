const ALGOS = ["bfs","dfs","greedy","astar"];

function drawGridToCanvas(canvas, grid, visited, path) {
    const ctx = canvas.getContext("2d");
    const rows = grid.length;
    const cols = grid[0].length;
    const cell = Math.floor(Math.min(canvas.width/cols, canvas.height/rows));
    ctx.fillStyle="#111"; ctx.fillRect(0,0,canvas.width,canvas.height);
    for (let r=0;r<rows;r++){
        for (let c=0;c<cols;c++){
            const ch = grid[r][c];
            ctx.fillStyle = ch === '#' ? "#000" : (ch==='S'?"#0f0": ch==='G'?"#f00":"#ffffffff");
            ctx.fillRect(c*cell, r*cell, cell, cell);
        }
    }
    ctx.fillStyle="rgba(68,68,255,0.45)"; for (let p of visited) ctx.fillRect(p.c*cell, p.r*cell, cell, cell);
    ctx.fillStyle="rgba(255,255,0,0.9)"; for (let p of path) ctx.fillRect(p.c*cell, p.r*cell, cell, cell);
}

function getMazeFromPath(){ const parts = location.pathname.split("/"); return decodeURIComponent(parts[parts.length-1]); }

async function loadAll(){
    const maze = getMazeFromPath();
    document.getElementById("mazeTitle").textContent = maze;
    const grid = document.getElementById("algoGrid");
    grid.innerHTML = "";
    for (const algo of ALGOS) {
        const col = document.createElement("div"); col.className = "col-md-6";
        const card = document.createElement("div"); card.className = "maze-card";
        const title = document.createElement("div"); title.textContent = algo.toUpperCase(); title.style.marginBottom="8px";
        const cv = document.createElement("canvas"); cv.width=500; cv.height=500;
        const openBtn = document.createElement("a"); openBtn.className="btn btn-sm btn-light mt-2"; openBtn.textContent="Open"; openBtn.href=`/run/${encodeURIComponent(maze)}/${algo}`;
        card.appendChild(title); card.appendChild(cv); card.appendChild(openBtn); col.appendChild(card); grid.appendChild(col);
        (async function(canvasEl, a){
            const resp = await fetch(`/api/run/${a}/${encodeURIComponent(maze)}`);
            if (!resp.ok) return;
            const data = await resp.json();
            drawGridToCanvas(canvasEl, data.grid, data.visited, data.path);
        })(cv, algo);
    }
}

document.addEventListener("DOMContentLoaded", loadAll);