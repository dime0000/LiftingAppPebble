

// PRESENTATION: basic method that takes a URL, get/post, 
var xhrRequest = function (url, type, callback) {
  
  var xhr = new XMLHttpRequest();
  
  xhr.onload = function () {
    callback(this.responseText);
  };
  
  xhr.open(type, url);
  
  xhr.send();
  
};



// PRESENTATION: main method - uses a web api that returns JSON
function getExercises(exercise_key) {

  // Construct URL - change to the exercises URL..
  // something wrong with the URL!
  
// PRESENTATION: URL
  var url = exercise_key;

  console.log('Running getExercises');

  // Send request 
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info

      
      // PRESENTATION: parses the json results
      //console.log(JSON.parse(responseText));
       
      var json = JSON.parse(responseText);
      var data = json.results;
      var exercisename = "";
      
      console.log('about to loop');
      
      if (json.previous !== null && json.previous !== "null") {
        exercisename = exercisename + json.previous + "|";
      }
      
      // PRESENTATION: loops through, puting the JSON "name" key in to delimited string
      // should go through results array!
      for(var i in data)
      {
          if (data[i].language == "2" && data[i].status == "2") {
            exercisename = exercisename + data[i].name + "|";
          }
      }
      if (json.next !== null && json.next !== "null") {
        exercisename = exercisename + json.next + "|";
      }
      
     
      // Assemble dictionary using our keys
      // determine how to send via another array
      
      // PRESENTATION: pushes string the dictionary using known key
      var dictionary = {
        'EXERCISE_NAME': exercisename
      };
      
      // Send to Pebble
      
      // PRESENTATION: sends the message back to pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log('info sent to Pebble successfully!');
        },
        function(e) {
          console.log('Error sending info to Pebble!');
        }
      );
      

    }      
  );
}


// PRESENTATION: handler when the app loads (we're not doing anything for this app on load)
// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');
    
    //getExercises();
  }
);


// PRESENTATION: this handler when it gets a message from the watch
// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    // should only get exercises for key 0... key 1 just goes right to android
    var exercise_key = e.payload["EXERCISE_NAME"];
    
    console.log("AppMessage received!--> " + JSON.stringify(e));
    if (exercise_key !== undefined) {
        // Show the parsed response
      console.log("GOOD HERE-->" + exercise_key);
      getExercises(exercise_key);
    } else {
        // Show the parsed response
      console.log("WHAT DO I DO HERE??");
    }
  }                     
);