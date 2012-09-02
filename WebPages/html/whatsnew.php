<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
 <link rel="stylesheet" href="../style.css" type="text/css">	
</head>
<body>

<div id="TopMenu">
	<a href="../index.html">Home</a> | <a href="whatsnew.php">What&apos;s New?</a> | <a href="software.php">Software</a> | <a href="https://plus.google.com/photos/107287498294316622336/albums/5758446343165527137">BigTrak</a>
</div>

<div style="padding: 1em 0px 0px 0px;"></div>

<?php
$link = @mysql_connect("db1748.oneandone.co.uk", "dbo274106622", "5MqwnprJ") or
die(mysql_error());

if(!mysql_select_db("db274106622", $link)) {
  echo("Failed to open database");
}
?>

  <h1>What&apos;s new</h1>

<?php
$result = mysql_query("SELECT * FROM WhatsNew WHERE 1 ORDER BY DATE DESC", $link);

while ($row = mysql_fetch_array($result, MYSQL_ASSOC)) {
    echo("" . $row["Date"] . " " . $row["Entry"] ."<br />");
}
 
?>

</body>


<?php
mysql_close($link);
?>

</html>
