// test.js
var addon = require('wickrio_addon');
var fs = require('fs');
module.exports = addon;

return new Promise( (resolve, reject) => {
  if (process.argv[2] === undefined) {
    var client = fs.readFileSync('client_bot_username.txt', 'utf-8');
    client = client.trim();
    var response = addon.clientInit(client);
    resolve(response);
  } else {
    var response = addon.clientInit(process.argv[2]);
    resolve(response);
  }

}).then(result => {
  console.log(result);

// addon(printer);

console.log(addon.cmdStartAsyncRecvMessages('printer'));
function printer(message){
  console.log('message:', message);
}
}).catch(error => {
  console.log('Error: ', error);
});
