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
	res.send(clients);
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

app.put('/', function (req, res) {
	var client = {
		uuid: uuid.v4()
	};
	clients[client.uuid] = client;
	res.send(client.uuid);
});

app.del('/:uuid', function (req, res) {
	var removeUuid = req.params.uuid;
	var client = clients[removeUuid];
	
	if( !client ) {
		res.send('Could not find client ' + removeUuid);
		return;
	}
	
	// Make sure we do not have any references anymore
	client.uuid = undefined;
	clients[removeUuid] = undefined;
	
	res.send('Removed');
});


// Launch server

app.listen(4242);

