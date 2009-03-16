<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
</head>
<body>

<?php
$link = @mysql_connect("db1748.oneandone.co.uk", "dbo274106622", "5MqwnprJ") or
die(mysql_error());

if(!mysql_select_db("db274106622", $link)) {
  echo("Failed to open database");
}
?>

  <h1>Software 'Wot I have written</h1>

  <p>The following programs are written for computers running <a href="http://www.riscos.com">RISC OS<a/>. These can either be run on a real 
    piece of RISC OS hardware, or on one of its emulators.</a>
  </p>

<?php
$result = mysql_query("SELECT * FROM Software", $link);

while ($row = mysql_fetch_array($result, MYSQL_ASSOC)) {
    echo("<dl>");
    echo("<dt><a href=\"" . $row["Location"] . "\">");
   
    // Do we have an image to display?
    if("" != $row["Icon"]) {
      echo("<img border=\"0\" src=\"" . $row["Icon"] . "\">");
    }

    echo($row["Name"] . "</a> Updated " . $row[Changed] . "</dt>");
    echo("<dd>" . $row["Description"] . "</dd>");
    echo("</dl>");
}
 
?>

</body>


<?php
mysql_close($link);
?>

</html>
