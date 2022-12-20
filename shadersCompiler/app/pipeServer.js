var net = require('net');

var PIPE_NAME = "mynamedpipe";
var PIPE_PATH = "\\\\.\\pipe\\" + PIPE_NAME;

var L = console.log;

var server = net.createServer(function(stream) {
    L('Server: on connection')

    stream.on('end', function() {
        document.app.pipeServer.sendMessage = function(message) {
            console.log("No connection!");
        };
    });

    document.app.pipeServer.sendMessage = function(message) {
        stream.write(message);
    };
});

server.on('close',function(){
    L('Server: on close');
})

server.listen(PIPE_PATH,function(){
    L('Server: on listening');
})

/*
// == Client part == //
var client = net.connect(PIPE_PATH, function() {
    L('Client: on connection');
})

client.on('data', function(data) {
    L('Client: on data:', data.toString());
    client.end('Thanks!');
});

client.on('end', function() {
    L('Client: on end');
})
*/


if (!document.app) {
    document.app = {};
}

document.app.pipeServer = {
    sendMessage: function(message) {
        console.log("No connection!");
    }
};
