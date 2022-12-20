const settingsFile = path.join(__dirname, '../../../../data/scripts/clouds/settings.json');

function loadSettings(onReady) {
    fs.readFile(settingsFile, function(err, data) {
        let str = data.toString();
        let obj = JSON.parse(str);

        onReady(obj);
    });
}

function storeSettings(obj, onReady) {
    fs.writeFile(settingsFile, JSON.stringify(obj, null, 4), function() {
        onReady();
    });
}

if (!document.app) {
    document.app = {};
}
document.app.settings = {
    loadSettings: loadSettings,
    storeSettings: storeSettings,
};

