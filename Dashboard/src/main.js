var DATA = {},
SENSORS = [];
DROOG=176,
ZWAAILICHT=false;
// Create a client instance
var client = new Paho.MQTT.Client("gost.geodan.nl", 9001, new Date().getTime()+navigator.userAgent);

// set callback handlers
client.onConnectionLost = onConnectionLost;
client.onMessageArrived = onMessageArrived;

// connect the client
client.connect({onSuccess:onConnect});


// called when the client connects
function onConnect() {
  // Once a connection has been made, make a subscription and send a message.
  console.log("onConnect");
  client.subscribe("Datastreams(32)/Observations");
  client.subscribe("Datastreams(35)/Observations");
  client.subscribe("hierisanne");
  message = new Paho.MQTT.Message("{'task':'waar is anne'}");
  message.destinationName = "waarisanne";
  client.send(message);
  d3.json('http://gost.geodan.nl/v1.0/Datastreams(35)/Observations?$top=1',function(d){
    zwaaiLicht(d.value[0]);
  })
}

// called when the client loses its connection
function onConnectionLost(responseObject) {
  if (responseObject.errorCode !== 0) {
    console.log("onConnectionLost:"+responseObject.errorMessage);
    setTimeout(function () {
      client.connect({onSuccess:onConnect});
    },1000)
  }

  
}

// called when a message arrives
function onMessageArrived(message) {
  var obj = JSON.parse(message.payloadString)  
  switch(message.destinationName){
    case "Datastreams(32)/Observations":
      happyPlant(obj,'one');
      break;
    case "Datastreams(35)/Observations":
      zwaaiLicht(obj);
      break;
    case "hierisanne":
      hierIsAnne(obj);
      break;
  }
}

function happyPlant(msg,sensor) {
  if( DATA[sensor] === undefined) DATA[sensor] = [];
  DATA[sensor].push({value:msg.result,time:new Date(msg.phenomenonTime)})
  var droog = msg.result>DROOG?true:false;
  var sensId = SENSORS.indexOf(sensor);
  if(sensId<0) {
    SENSORS.push(sensor);
    sensId = SENSORS.length-1;
    var sensorDiv = d3.select("#vrolijkeFransOverview")
    .append('div')
    .attr('id',sensor);


    sensorDiv
    .append('div')
    .classed('plant tile',true)
    

  }
  d3.select('#'+sensor)
  .select('.plant')
  .classed('droog',droog)
  .text(msg.result)

}

function hierIsAnne(msg) {
  if(msg.location === null) {
  var sensorDiv = d3.select("#anneStatus")
    .text('Nee!')
  }
  else {
    var sensorDiv = d3.select("#anneStatus")
    .text('Misschien');
    var textDiv = d3.select('#anneText')
    .html('');
    textDiv
    .append('div')
    .classed('actie',true)
    .text('Anne is waarschijnlijk aan het '+msg.action);
    textDiv
    .append('div')    
    .html('in <span class="locatie">'+msg.location+'</span>')
  }

}
function zwaaiLicht(msg) {
  var zwaaiDiv = d3.select('#zwaaiLicht')
    .html('');
  var text = 'zwaait';
  if(msg.result) { 
    zwaaiDiv.append('div').classed('zwaaien',true)
  }
  else {
   zwaaiDiv.text('zwaait niet');
 }

  
}