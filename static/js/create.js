let rows = 15;
let cols = 15;

let grid = [];          // '.' empty, '#' wall, 'S', 'G'
let mode = "wall";      // "wall", "start", "goal"

let generating = false; // prevents redraw during generation

const canvas = document.getElementById("editorCanvas");
const ctx = canvas.getContext("2d");

// ---------------------- GRID INIT ----------------------

function initGrid() {
    grid = Array.from({ length: rows }, () =>
        Array.from({ length: cols }, () => '.')
    );
}

// ---------------------- MAZE GENERATOR (DFS, 2-step) ----------------------

function generateRandomMazeDFS_grid() {
    generating = true;

    // Make everything wall
    grid = Array.from({ length: rows }, () =>
        Array.from({ length: cols }, () => '#')
    );

    function inside(r, c) {
        return r > 0 && r < rows - 1 && c > 0 && c < cols - 1;
    }

    const stack = [];

    // pick a random odd cell
    let r = (Math.floor(Math.random() * (rows / 2)) * 2) + 1;
    let c = (Math.floor(Math.random() * (cols / 2)) * 2) + 1;

    if (r >= rows) r = rows - 2;
    if (c >= cols) c = cols - 2;

    grid[r][c] = '.';
    stack.push([r, c]);

    while (stack.length > 0) {
        const [cr, cc] = stack[stack.length - 1];

        const dirs = [
            [-2, 0],
            [2, 0],
            [0, -2],
            [0, 2]
        ];

        // shuffle dirs
        for (let i = dirs.length - 1; i > 0; i--) {
            const j = Math.floor(Math.random() * (i + 1));
            [dirs[i], dirs[j]] = [dirs[j], dirs[i]];
        }

        let carved = false;

        for (const [dr, dc] of dirs) {
            const nr = cr + dr;
            const nc = cc + dc;

            if (inside(nr, nc) && grid[nr][nc] === '#') {
                // carve the between cell
                grid[cr + dr / 2][cc + dc / 2] = '.';
                grid[nr][nc] = '.';
                stack.push([nr, nc]);
                carved = true;
                break;
            }
        }

        if (!carved) {
            stack.pop();
        }
    }

    // Remove any old S/G
    for (let i = 0; i < rows; i++)
        for (let j = 0; j < cols; j++)
            if (grid[i][j] === 'S' || grid[i][j] === 'G') grid[i][j] = '.';

    // Pick S and G on empty cells: top-left & bottom-right region
    function findEmptyNear(r, c) {
        if (grid[r][c] === '.') return [r, c];

        const q = [[r, c]];
        const seen = Array.from({ length: rows }, () => Array(cols).fill(false));
        seen[r][c] = true;

        const deltas = [[1,0],[-1,0],[0,1],[0,-1]];
        let qi = 0;

        while (qi < q.length) {
            const [rr, cc] = q[qi++];
            for (const [dr, dc] of deltas) {
                const nr = rr + dr, nc = cc + dc;
                if (nr >= 0 && nr < rows && nc >= 0 && nc < cols && !seen[nr][nc]) {
                    if (grid[nr][nc] === '.') return [nr, nc];
                    seen[nr][nc] = true;
                    q.push([nr, nc]);
                }
            }
        }
        return [r, c];
    }

    let [sr, sc] = findEmptyNear(1, 1);
    grid[sr][sc] = 'S';

    let [gr, gc] = findEmptyNear(rows - 2, cols - 2);
    if (gr === sr && gc === sc) {
        // find anything else
        outer:
        for (let i = rows - 2; i >= 1; i--) {
            for (let j = cols - 2; j >= 1; j--) {
                if (grid[i][j] === '.') {
                    gr = i; gc = j;
                    break outer;
                }
            }
        }
    }

    grid[gr][gc] = 'G';

    generating = false;
    drawGrid();
}

// ---------------------- CANVAS ----------------------

function resizeCanvas() {
    const wrap = document.getElementById("canvasWrap");
    const rect = wrap.getBoundingClientRect();
    const size = Math.min(rect.width - 20, 600);
    const dpr = window.devicePixelRatio || 1;

    canvas.style.width = `${size}px`;
    canvas.style.height = `${size}px`;
    canvas.width = size * dpr;
    canvas.height = size * dpr;

    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
}

function drawGrid() {
    if (generating) return;

    resizeCanvas();

    const dpr = window.devicePixelRatio || 1;
    const cell = canvas.width / dpr / cols;

    ctx.fillStyle = "#111";
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    for (let r = 0; r < rows; r++) {
        for (let c = 0; c < cols; c++) {
            const ch = grid[r][c];

            // base
            ctx.fillStyle = (ch === '#') ? "#000" : "#ffffffff";
            ctx.fillRect(c * cell, r * cell, cell, cell);

            if (ch === "S") {
                ctx.fillStyle = "#2ecc71";
                ctx.fillRect(c * cell, r * cell, cell, cell);
            }
            if (ch === "G") {
                ctx.fillStyle = "#e74c3c";
                ctx.fillRect(c * cell, r * cell, cell, cell);
            }

            // border
            ctx.strokeStyle = "#2e2e2e";
            ctx.lineWidth = 0.7;
            ctx.strokeRect(
                c * cell + 0.35,
                r * cell + 0.35,
                cell - 0.7,
                cell - 0.7
            );
        }
    }
}

// ---------------------- CLICK HANDLING ----------------------

function canvasClick(e) {
    const rect = canvas.getBoundingClientRect();
    const px = e.clientX - rect.left;
    const py = e.clientY - rect.top;

    const dpr = window.devicePixelRatio || 1;
    const cell = canvas.width / dpr / cols;

    const c = Math.floor(px / cell);
    const r = Math.floor(py / cell);

    if (r < 0 || r >= rows || c < 0 || c >= cols) return;

    if (mode === "wall") {
        grid[r][c] = (grid[r][c] === "#") ? "." : "#";
    }
    else if (mode === "start") {
        for (let i = 0; i < rows; i++)
            for (let j = 0; j < cols; j++)
                if (grid[i][j] === "S") grid[i][j] = ".";
        grid[r][c] = "S";
    }
    else if (mode === "goal") {
        for (let i = 0; i < rows; i++)
            for (let j = 0; j < cols; j++)
                if (grid[i][j] === "G") grid[i][j] = ".";
        grid[r][c] = "G";
    }

    drawGrid();
}

// ---------------------- SAVE ----------------------

async function saveMaze() {
    const filename = document.getElementById("fileName").value.trim();
    if (!filename.endsWith(".txt")) {
        alert("Filename must end with .txt");
        return;
    }

    let content = "";
    for (let r = 0; r < rows; r++)
        content += grid[r].join("") + "\n";

    const res = await fetch("/api/maze/upload", {
        method: "POST",
        body: JSON.stringify({ filename, contents: content })
    });

    if (res.ok) {
        alert("Maze saved!");
        window.location.href = "/mazes";
    } else {
        alert("Failed to save maze");
    }
}

// ---------------------- EVENTS ----------------------

document.getElementById("applySize").onclick = () => {
    rows = parseInt(document.getElementById("rowsInput").value);
    cols = parseInt(document.getElementById("colsInput").value);
    initGrid();
    drawGrid();
};

document.getElementById("setStart").onclick = () => mode = "start";
document.getElementById("setGoal").onclick = () => mode = "goal";
document.getElementById("randomMaze").onclick = generateRandomMazeDFS_grid;
document.getElementById("saveMaze").onclick = saveMaze;

canvas.addEventListener("click", canvasClick);

window.addEventListener("resize", () => {
    if (!generating) drawGrid();
});

// ---------------------- INIT ----------------------

initGrid();
drawGrid();