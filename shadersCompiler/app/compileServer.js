const path = require('path');
const fs = require('fs');
const crypto = require('crypto');
const cp = require('child_process');

const shadersDir = path.join(__dirname, '../../../../data/scripts/shaders');

function getShaders(onReady) {
    fs.readdir(shadersDir, function (err, files) {
        if (err) {
            return console.log('Unable to scan directory: ' + err);
        }
        
        const re = /.*\.hlsl$/;
        files = files.filter((file) => {
            return re.test(file);
        });
        onReady(files);
    });
}

function loadFile(file, onReady) {
    fs.readFile(path.join(shadersDir, file), function(err, data) {
        data = data.toString();
        onReady(data);
    });
}

function getFileInfo(file, onReady) {
    const vs = /VSMain/;
    const ps = /PSMain/;
    const cs = /CSMain/;

    loadFile(file, function(code) {
        let hash = crypto.createHash('md5').update(code).digest('hex');
        onReady({
            name: file,
            hash: hash,
            vs: vs.test(code),
            ps: ps.test(code),
            cs: cs.test(code),
        });
    });
}

function runCompiler(cmd, onReady) {
    let pr = cp.exec(cmd, {shell: true}, function(err, data) {
        onReady(err);
    });

    return pr;
}

if (!document.app) {
    document.app = {};
}
document.app.compileServer = {
    getShaders: getShaders,
    getFileInfo: getFileInfo,
    fxc: path.join(__dirname, 'fxc.exe'),
    shaders: shadersDir,
    path: path,
    runCompiler: runCompiler
};
