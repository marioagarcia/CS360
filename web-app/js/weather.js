// $( "#city-input" ).keyup(function(event) {
//     alert("Keyup " + $("#city-input").val());
// });

$( "#city-input" ).keyup(function() {
  var rest_url = "http://students.cs.byu.edu/~clement/CS360/ajax/getcity.cgi?q=" + $("#city-input").val();
  $.getJSON(rest_url, function(data) {

    var everything = "";

    $.each(data, function(i,item) {
      everything += "<a href=\"\" class=\"list-group-item\">" + data[i].city + "</a>";
    });

    if(everything != "") {
      $("#suggestions").addClass("list-group-item");
      $("#suggestions").html(everything);
    }
    else {
      $("#suggestions").html("");
      $("#suggestions").removeClass("list-group-item");
    }

    console.log("did it!");

  })
  .done(function() { console.log('getJSON request succeeded!'); })
  .fail(function(jqXHR, textStatus, errorThrown) {
    console.log('getJSON request failed! ' + textStatus);
    console.log("incoming "+jqXHR.responseText);
    $("#suggestions").html("");
    $("#suggestions").removeClass("list-group-item");
  })
  .always(function() { console.log('getJSON request ended!');
})
.complete(function() { console.log("complete"); });
});


$("#city-btn").click(function(e) {

  var city = $("#city-input").val();

  var wu_url = "https://api.wunderground.com/api/d91c5cfe30b71852/geolookup/conditions/q/UT/";
  wu_url += city;
  wu_url += ".json";
  console.log(wu_url);
  $.ajax({
    url : wu_url,
    dataType : "jsonp",
    success : function(parsed_json) {
      var location = parsed_json['location']['city'];
      var temp_string = parsed_json['current_observation']['temperature_string'];
      var current_weather = parsed_json['current_observation']['weather'];
      everything = "<ul>";
      everything += "<li>Temperature: "+temp_string;
      everything += "<li>Weather: "+current_weather;
      everything += "</ul>";
      $("#weather-pnl-title").html(location);
      $("#weather-pnl-body").html(everything);
    }
  });

  $("#chosen-city").val(city);
  e.preventDefault();

});
