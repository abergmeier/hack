var application_root = __dirname,
	express = require("express"),
	uuid = require('node-uuid');

var app = express.createServer();

// Config

app.configure(function () {
	app.use(express.bodyParser());
	app.use(express.methodOverride());
	app.use(app.router);
	app.use(express.errorHandler({ dumpExceptions: true, showStack: true }));
});

var clients = {};

app.get('/', function (req, res) {
	var result = [];
	for( var client in clients ) {
		if( !clients.hasOwnProperty(client) )
			continue;
		result.push( clients[client] );
	}
	res.send(result);
});

app.get('/:uuid', function (req, res) {
	var removeUuid = req.params.uuid;
	var client = clients[removeUuid];
	
	if( !client ) {
		res.send('Could not find client ' + removeUuid);
		return;
	}
	
	res.send(client);
});

app.post('/:uuid', function (req, res) {
	var client = {
		uuid: req.params.uuid,
		host: req.body.host,
		port: req.body.port
	};
	
	clients[client.uuid] = client;
	console.log("Added: " + client.uuid + " " + client.host + ":" + client.port);
	res.send("");
});

app.del('/:uuid', function (req, res) {
	var removeUuid = req.params.uuid;
	var client = clients[removeUuid];
	
	if( !client ) {
		res.send('Could not find client ' + removeUuid);
		return;
	}
	
	console.log("Removed: " + client.uuid + " " + client.host + ":" + client.port);
	
	// Make sure we do not have any references anymore
	client.uuid = undefined;
	delete clients[removeUuid];
	
	res.send("");
});


// Launch server

app.listen(4242);

