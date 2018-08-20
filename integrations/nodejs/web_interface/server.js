const express = require('express');
const MongoClient = require('mongodb').MongoClient;
const bodyParser = require('body-parser');
const addon = require('wickrio_addon');
const fs = require('fs');
const app = express();

const port = 8090;
var api_key = process.env.API_KEY;

return new Promise(async (resolve, reject) => {
  if (process.argv[2] === undefined) {
    var client = await fs.readFileSync('client_bot_username.txt', 'utf-8');
    client = client.trim();
    var response = await addon.clientInit(client);
    resolve(response);
  } else {
    var response = await addon.clientInit(process.argv[2]);
    resolve(response);
  }

}).then(result => {
  console.log(result);
  app.use(bodyParser.json());

  app.listen(port, () => {
    console.log('We are live on ' + port);
  });

  var endpoint = "/Apps/" + api_key;

  app.post(endpoint + "/Messages", async function(req, res) {
    console.log('req.body:', req.body);
    res.send("OK 200");
    res.end();
    var ttl = "",
      bor = "";
    if (req.body.ttl)
      ttl = req.body.ttl.toString();
    if (req.body.bor)
      bor = req.body.bor.toString();
    if (req.body.users) {
      var users = [];
      for (var i in req.body.users) {
        users.push(req.body.users[i].name);
      }
      if (req.body.attachment) {
        var attachment;
        var displayName = "";
        if (req.body.attachment.url) {
          attachment = req.body.attachment.url.toString();
          displayName = req.body.attachment.displayname.toString();
        } else {
          attachment = req.body.attachment.filename.toString();
          displayName = req.body.attachment.displayname.toString();
        }
        console.log('users:', users);
        console.log('attachment:', attachment);
        console.log('displayName:', displayName);
        console.log(addon.cmdSend1to1Attachment(users, attachment, displayName, ttl, bor));
      } else {
        var message = req.body.message;
        var csm = addon.cmdSend1to1Message(users, message, ttl, bor);
        console.log(csm);
      }
    } else if (req.body.vgroupid) {
      var vGroupID = req.body.vgroupid.toString();
      if (req.body.attachment) {
        var attachment;
        var displayName = "";
        if (req.body.attachment.url) {
          attachment = req.body.attachment.url.toString();
          displayName = req.body.attachment.displayname.toString();
        } else {
          attachment = req.body.attachment.filename.toString();
          displayName = req.body.attachment.displayname.toString();
        }
        console.log('attachment:', attachment);
        console.log('displayName:', displayName);
        console.log(addon.cmdSendRoomAttachment(vGroupID, attachment, displayName, ttl, bor));
      } else {
        var message = req.body.message;
        console.log(addon.cmdSendRoomMessage(vGroupID, message, ttl, bor));
      }
    }
  });


  app.get(endpoint + "/Statistics", async function(req, res) {
    var statistics = await addon.cmdGetStatistics();
    res.send(statistics);
  });


  app.delete(endpoint + "/Statistics", async function(req, res) {
    var statistics = await addon.cmdClearStatistics();
    res.send("statistics cleared successfully");
  });


  app.post(endpoint + "/Rooms", async function(req, res) {
    console.log('req.body:', req.body);
    var room = req.body.room;
    var title = room.title.toString();
    var description = room.description.toString();
    var ttl = "",
      bor = "";
    if (room.ttl)
      ttl = room.ttl.toString();
    if (room.bor)
      bor = room.bor.toString();
    var members = [],
      masters = [];
    for (var i in room.members) {
      members.push(room.members[i].name);
    }

    for (var i in room.masters) {
      masters.push(room.masters[i].name);
    }
    var car = await addon.cmdAddRoom(members, masters, title, description, ttl, bor);
    console.log(car);
    res.send(car);
  });

}).catch(error => {
  console.log('Error: ', error);
});
