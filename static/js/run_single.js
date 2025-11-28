// run_single.js - improved: reads path, responsive canvas, crisp drawing, animation
function getPathParamsFromUrl() {
    const parts = window.location.pathname.split("/").filter(Boolean);
    // expected ... /run/<maze>/<algo>
    if (parts.length < 3) return null;
    const algo = parts.pop();
    const maze = parts.pop();
    return { algo: decodeURIComponent(algo), maze: decodeURIComponent(maze) };
}

async function fetchRun(algo, maze) {
    const resp = await fetch(`/api/run/${algo}/${encodeURIComponent(maze)}`);
    if (!resp.ok) throw new Error("run failed");
    return await resp.json();
}

function drawBase(ctx, grid) {
    const rows = grid.length;
    const cols = grid[0].length;
    const cell = Math.floor(ctx.canvas.width / cols);

    // base background
    ctx.fillStyle = "#111";
    ctx.fillRect(0,0,ctx.canvas.width,ctx.canvas.height);

    // draw cells
    for (let r=0;r<rows;r++){
        for (let c=0;c<cols;c++){
            const ch = grid[r][c];
            if (ch === '#') ctx.fillStyle = "#000";
            else ctx.fillStyle = "#ffffffff";
            ctx.fillRect(c*cell, r*cell, cell, cell);

            // thin border
            ctx.strokeStyle = "#2b2b2b";
            ctx.lineWidth = 0.7;
            ctx.strokeRect(c*cell + 0.5, r*cell + 0.5, cell - 1, cell - 1);
        }
    }
}

function drawStartGoal(ctx, grid) {
    const rows = grid.length;
    const cols = grid[0].length;
    const cell = Math.floor(ctx.canvas.width / cols);

    for (let r=0;r<rows;r++){
        for (let c=0;c<cols;c++){
            const ch = grid[r][c];
            if (ch === 'S') {
                ctx.fillStyle = "#2ecc71"; // bright green
                ctx.fillRect(c*cell, r*cell, cell, cell);
            } else if (ch === 'G') {
                ctx.fillStyle = "#e74c3c"; // red
                ctx.fillRect(c*cell, r*cell, cell, cell);
            }
        }
    }
}

async function renderAndAnimate(algo, maze, speed) {
    const data = await fetchRun(algo, maze);
    const grid = data.grid, visited = data.visited || [], path = data.path || [];
    const canvas = document.getElementById("mainCanvas");
    const ctx = canvas.getContext("2d");

    drawBase(ctx, grid);
    drawStartGoal(ctx, grid);

    // animate visited
    ctx.fillStyle = "rgba(68,68,255,0.6)";
    const cell = Math.floor(canvas.width / grid[0].length);
    for (let i=0;i<visited.length;i++){
        const p = visited[i];
        ctx.fillRect(p.c * cell, p.r * cell, cell, cell);
        await new Promise(r => setTimeout(r, speed));
    }

    // animate path
    ctx.fillStyle = "rgba(255,215,0,0.95)";
    for (let i=0;i<path.length;i++){
        const p = path[i];
        ctx.fillRect(p.c * cell, p.r * cell, cell, cell);
        await new Promise(r => setTimeout(r, Math.max(6, speed)));
    }

    // redraw start/goal to ensure visibility
    drawStartGoal(ctx, grid);
}

document.addEventListener("DOMContentLoaded", async () => {
    const params = getPathParamsFromUrl();
    if (!params) {
        console.error("Invalid run URL. Expected /run/<maze>/<algo>");
        return;
    }
    const algo = params.algo;
    const maze = params.maze;
    document.getElementById("title").textContent = `${algo.toUpperCase()} — ${maze}`;

    const speedSel = document.getElementById("speedSel");
    const rerun = document.getElementById("rerunBtn");

    // first render
    try {
        await renderAndAnimate(algo, maze, parseInt(speedSel.value));
    } catch (e) {
        console.error(e);
        alert("Failed to run algorithm: " + e.message);
    }

    rerun.addEventListener("click", async () => {
        try {
            await renderAndAnimate(algo, maze, parseInt(speedSel.value));
        } catch (e) {
            console.error(e);
            alert("Failed to run algorithm: " + e.message);
        }
    });

    // handle resize so canvas stays crisp/responsive
    window.addEventListener("resize", () => {
        // re-render static base (no animation)
        // simply re-run without animation delay (speed = 0)
        renderAndAnimate(algo, maze, 0).catch(()=>{});
    });
});