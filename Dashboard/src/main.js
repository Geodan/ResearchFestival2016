var DATA = {},
SENSORS = [];
DROOG=550,
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
  client.subscribe("Datastreams(36)/Observations");
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
    case "Datastreams(36)/Observations":
      iBeacon(obj);
      break;
    case "hierisanne":
      hierIsAnne(obj);
      break;
  }
}

function happyPlant(msg,sensor) {  
  if( DATA[sensor] === undefined) {
    DATA[sensor] = [];
    d3.select('#emptyVrolijkeFrans').remove();
    createLine(sensor)
  }
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
    .classed('nee',true)
    .text('Nee!');
    var textDiv = d3.select('#anneText')
    .html('');
    if(msg.action === undefined || msg.action === null) {
      textDiv
      .append('div')
      .classed('actie',true)
      .text('vragen heeft dus geen zin');
    }
    else {
       textDiv
      .append('div')
      .classed('actie',true)
      .html('Anne is waarschijnlijk bezig met: </div><div class="locatie">'+msg.action +'</div><div> probeer het daar eens');
    }
   
  }
  else {
    var sensorDiv = d3.select("#anneStatus")
    .classed('nee',false)
    .text('Misschien');
    var textDiv = d3.select('#anneText')
    .html('');
    textDiv
    .append('div')
    .classed('actie',true)
    .text('Anne is waarschijnlijk bezig met '+msg.action);
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

function createLine(sensor) {
  var graphSpace = d3.select('#vrolijkeFransGrafiek').node().getBoundingClientRect();
  var margin = {top: 20, right: 20, bottom: 30, left: 50},
    width = graphSpace.width - margin.left - margin.right,
    height = 200 - margin.top - margin.bottom;
  var formatDate = d3.timeFormat("%d-%b-%y");

  var x = d3.scaleTime()
      .range([0, width]);

  var y = d3.scaleLinear()
      .range([height, 0]);

  var xAxis = d3.axisBottom(x);

  var yAxis = d3.axisLeft(y);

  var line = d3.line()
      .x(function(d) { return x(d.date); })
      .y(function(d) { return y(d.close); });

  var svg = d3.select("#vrolijkeFransGrafiek").append("svg")
    .attr("width", width + margin.left + margin.right)
    .attr("height", height + margin.top + margin.bottom)
  .append("g")
    .attr("transform", "translate(" + margin.left + "," + margin.top + ")");
  x.domain(d3.extent(DATA[sensor], function(d) { return d.time; }));
  y.domain(d3.extent(DATA[sensor], function(d) { return d.value; })); 

  svg.append("g")
      .attr("class", "x axis")
      .attr("transform", "translate(0," + height + ")")
      .call(xAxis);

  svg.append("g")
      .attr("class", "y axis")
      .call(yAxis)
    .append("text")
      .attr("transform", "rotate(-90)")
      .attr("y", 6)
      .attr("dy", ".71em")
      .style("text-anchor", "end")
      .text("Price ($)");

  svg.append("path")
      .datum(DATA[sensor])
      .attr("class", "line")
      .attr("d", line);
}

function iBeacon(msg) {
    var res = msg.result;
    var name = res.split('_')[0];
    var near = res.split('_')[1];
    var text;
    if (near === '1') {
      text = name + '\'s telefoon is dichtbij';
    }
    else {
      text = name + '\'s telefoon is niet dichtbij'
    }
    
    d3.select('#iBeacon').text(text);
}
