var addon = require('bindings')('wickrio_addon');

module.exports = addon;
console.log(addon.clientInit('aaronbot019512@62114373.net'));
var vGroupID = "Sbfa682074f470f8cfe7cf05060559918c798bb4fe5355531e84340694b367a6";
var members = ['wickraaron@wickrautomation.com', 'wickrpak@wickrautomation.com'];
var members1;
var moderators = ['wickraaron@wickrautomation.com', 'aaronbot019512@62114373.net'];
var bor = "6";
var ttl = "100";
var title = "West side";
var description = "room";
var message = "Testing time!"
//console.log(addon.cmdAddRoom(members, moderators, title, description, ttl, bor));
//console.log(addon.cmdDeleteRoom(vGroupID));
//console.log(addon.cmdGetStatistics());
//console.log(addon.cmdClearStatistics());
//console.log(addon.cmdGetRoom(vGroupID));
//console.log(addon.cmdGetRooms());
//console.log(addon.cmdLeaveRoom(vGroupID));
//console.log(addon.cmdAddGroupConvo(members, ttl, bor));
//console.log(addon.cmdModifyRoom(vGroupID, members, moderators, title, description, ttl, bor));
//console.log(addon.cmdGetGroupConvo(vGroupID));
//console.log(addon.cmdGetGroupConvos());
//console.log(addon.cmdDeleteGroupConvo(vGroupID));
// console.log(addon.cmdGetReceivedMessage());
console.log(addon.cmdSendMessage(vGroupID, members, message, ttl , bor));
