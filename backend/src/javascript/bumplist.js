UPDATE logic SET logic = '
/*
 * BumpList implemented in Javascript to run on Thwonk
 *
 * Author: Mike Bennett (mike@thwonk.com)
*/

/*
 * C like include of ParseMail object for parsing Mail headers
*/
eval(Thwonk.file.read("/bumplist/code/ParseMail.js"));

/*
 * Variables and paths for user messages
*/
var BumplistAddress = "thelist@bumplist.net";

var BumpedMsgPath_P1 = "/bumplist/bumped_mail_p1.txt";
var BumpedMsgPath_P2 = "/bumplist/bumped_mail_p2.txt";
var SignupMsgPath_P1 = "/bumplist/signup_mail_p1.txt";
var SignupMsgPath_P2 = "/bumplist/signup_mail_p2.txt";
var TagLinePath = "/bumplist/tagline.txt";

/*
 * Locations for serialised javascript variables storing state information
*/
var SubscriberListPath = "/bumplist/subscribers.ser";
var SubjectsListPath = "/bumplist/recentsubjects.ser";
var BumpCountPath = "/bumplist/bumpcount.ser";
var MessageCountPath = "/bumplist/messagecount.ser";
var ResubCountPath = "/bumplist/resubcount.ser";

var SubscribersStatPath = "/bumplist/subscribers/";

var TopTimeStatsSer = "/bumplist/toptime.ser";
var TopPostsStatsSer = "/bumplist/topposts.ser";
var TopBumpsStatsSer = "/bumplist/topbumps.ser";

/*
 * Short HTML snipets for insertion into Bumplist website
*/
var StatsCornerHtml = "/bumplist/statshtml.html";
var FrontPageSubjectsHtml = "/bumplist/frontpagesubjects.html";
var StatsWhoOnHtml = "/bumplist/whoon.html";
var StatsTopTimeHtml = "/bumplist/toptime.html";
var StatsTopPostsHtml = "/bumplist/topposts.html";
var StatsTopBumpsHtml = "/bumplist/topbumps.html";


/*
 * Main entry point for Bumplist
*/
var msg = Thwonk.message.getCurrent();

if(ParseMail.getHeaderEntry("Subject:", msg).toLowerCase() == "subscribe") {
	subscribePerson(msg);
	generateStats();
} else {
/*
	sub = ParseMail.getHeaderEntry("Subject:", msg);

	dstring = String.replace(sub, /m/g, "michael");

	Thwonk.print("After: ", sub, " - ", dstring);
*/
	messageToSubscribers(msg);
	generateStats();
}


/*
 * Subject List Object
 *
 * sub = subject
 * dt = date time for subject
*/
function Object_SubjectListEntry(sub, dt, tlist) {
	this.subject = sub;
	this.dtime = dt;
	this.timeonlist = tlist;
}


/*
 * Subscriber Object
 *
 * uid = user name associated with this object
*/
function Object_Subscriber(uid) {
	var subFile;

	/* Constructor which create new Subscriber Object is user does not exist,
	 * otherwise loads in the users data that was previously stored */
	if((subFileTemp = Thwonk.file.read(SubscribersStatPath + uid)) == -1) {
		this.userId = uid;

		var curDate = new Date();
		this.subscribeTime = curDate.toLocaleString();

		this.totalTime = 0;		// Total time on bumplist
		this.numPosts = 0;
		this.numSubscribes = 0;
		this.numBumps = 0;
		this.numMessagesReceived = 0;
	} else {
		subFile = eval(subFileTemp);

		this.userId = subFile.userId;

		this.subscribeTime = subFile.subscribeTime;

		this.totalTime = subFile.totalTime;
		this.numPosts = subFile.numPosts;
		this.numSubscribes = subFile.numSubscribes;
		this.numBumps = subFile.numBumps;
		this.numMessagesReceived = subFile.numMessagesReceived;
	}
}
	

function figureTotalTime(person) {
	var tTime;
	var subIds = [];
	var subscribers = eval(Thwonk.file.read(SubscriberListPath));

	for(var i = 0; i < subscribers.length; i++) {
		subIds[i] = subscribers[i].userId;
	}

	// Is person currently subscribed?
	if(subIds.indexOf(person.userId) != -1) {
		// Yes
		tTime = new Date().getTime() - Date.parse(person.subscribeTime) + person.totalTime;
	} else {
		// No
		tTime = person.totalTime;
	}

	tTime = Math.floor((tTime / 1000) / 60);

	return tTime;
}

function sortBySubcriberName(first, second) {
	a = first.userId;
	b = second.userId;

	if(a < b) {
		return -1;
	}

	if(a > b) {
		return 1;
	}

	if(a == b) {
		return 0;
	}
}

function sortByTopTime(first, second) {
	a = first.totalTime;
	b = second.totalTime;

	if(a < b) {
		return -1;
	}

	if(a > b) {
		return 1;
	}

	if(a == b) {
		return 0;
	}
}

function sortByTopPosts(first, second) {
	a = first.numPosts;
	b = second.numPosts;

	if(a < b) {
		return -1;
	}

	if(a > b) {
		return 1;
	}

	if(a == b) {
		return 0;
	}
}

function sortByTopBumps(first, second) {
	a = first.numBumps;
	b = second.numBumps;

	if(a < b) {
		return -1;
	}

	if(a > b) {
		return 1;
	}

	if(a == b) {
		return 0;
	}
}
/*
 * Generate stats and laid out in html for website
*/
function generateStats() {
	/* Generate upper right corner stats, i.e. subscribes, bumped, re-subscribes */
//	var messageCount = eval(Thwonk.file.read(MessageCountPath));
	var bumpedCount = eval(Thwonk.file.read(BumpCountPath));
	var resubCount = eval(Thwonk.file.read(ResubCountPath));

	var cornerHtml = "<td width=140><center><font size=3 color=#ff0000 face=verdana>";
	cornerHtml += bumpedCount + 1;
	cornerHtml += "</font></center></td>";

	cornerHtml += "<td width=140><center><font size=3 color=#ff0000 face=verdana>";
	cornerHtml += bumpedCount;
	cornerHtml += "</font></center></td>";

	cornerHtml += "<td width=140><center><font size=3 color=#ff0000 face=verdana>";
	cornerHtml += resubCount;
	cornerHtml += "</font></center></td>";

	Thwonk.file.write(StatsCornerHtml, cornerHtml);

	/* Generate list of subjects for front page */
	var subjectsList = eval(Thwonk.file.read(SubjectsListPath));
	var frontSubHtml = "";

	subjectsList = subjectsList.reverse();

	for(var i = 0; i < subjectsList.length; i++) {
		frontSubHtml += "<tr align=left><td><font size=1 color=#000000 face=verdana>";
		frontSubHtml += subjectsList[i].dtime;
		frontSubHtml += "</font></td><td><font size=1 color=#000000 face=verdana>";
		frontSubHtml += subjectsList[i].timeonlist;
		frontSubHtml += " mins</font></td><td><font size=1 color=#000000 face=verdana>";
		frontSubHtml += subjectsList[i].subject;
		frontSubHtml += "</font></td></tr>";
	}

	Thwonk.file.write(FrontPageSubjectsHtml, frontSubHtml);

	var subscribers = eval(Thwonk.file.read(SubscriberListPath));
	var whoOnHtml = "";

	subscribers.sort(sortBySubcriberName);

	for(i = 0; i < subscribers.length; i++) {
		person = new Object_Subscriber(subscribers[i].userId);

		whoOnHtml += "<tr align=left><td><font size=1 color=#000000 face=verdana>";
		curSub = person.userId.split("@", 2);
		whoOnHtml += curSub[0];
		whoOnHtml += "</font></td><td><font size=1 color=#000000 face=verdana>";
		whoOnHtml += figureTotalTime(person); // Math.floor((person.totalTime / 1000) / 60);
		whoOnHtml += " mins</font></td><td><font size=1 color=#000000 face=verdana>";
		whoOnHtml += person.numPosts;
		whoOnHtml += "</font></td><td><font size=1 color=#000000 face=verdana>";
		whoOnHtml += person.numSubscribes;
		whoOnHtml += "</font></td><td><font size=1 color=#000000 face=verdana>";
		whoOnHtml += person.numBumps;
		whoOnHtml += "</font></td><td><font size=1 color=#000000 face=verdana>";

		if(person.numSubscribes != person.numBumps) {
			whoOnHtml += person.numBumps;
		} else {
			whoOnHtml += person.numSubscribes;
		}

		whoOnHtml += "</font></td></tr>";
	}

	Thwonk.file.write(StatsWhoOnHtml, whoOnHtml);

//	Thwonk.file.write(TopTimeStatsSer, subscribers.toSource());
//	Thwonk.file.write(TopPostsStatsSer, subscribers.toSource());
//	Thwonk.file.write(TopBumpsStatsSer, subscribers.toSource());

	generateStatsTopTime();
	generateStatsTopPosts();
	generateStatsTopBumps();
}


function generateStatsTopTime() {
	var i;
	var topTimeHtml = "";

	var topTimeList = eval(Thwonk.file.read(TopTimeStatsSer));

	topTimeList.sort(sortByTopTime);
	topTimeList.reverse();

	for(i = 0; i < topTimeList.length; i++) {
		person = new Object_Subscriber(topTimeList[i].userId);
		topTimeHtml += "<tr align=left><td><font size=1 color=#000000 face=verdana>";
		curSub = person.userId.split("@", 2);
		topTimeHtml += curSub[0];
		topTimeHtml += "</font></td><td><font size=1 color=#000000 face=verdana>";
		topTimeHtml += figureTotalTime(person); //Math.floor((person.totalTime / 1000) / 60);
		topTimeHtml += " mins</font></td></tr>";
	}

	Thwonk.file.write(StatsTopTimeHtml, topTimeHtml);
}


function generateStatsTopPosts() {
	var i;
	var topPostsHtml = "";

	var topPostsList = eval(Thwonk.file.read(TopPostsStatsSer));

	topPostsList.sort(sortByTopPosts);
	topPostsList.reverse();

	for(i = 0; i < topPostsList.length; i++) {
		person = new Object_Subscriber(topPostsList[i].userId);
		topPostsHtml += "<tr align=left><td><font size=1 color=#000000 face=verdana>";
		curSub = person.userId.split("@", 2);
		topPostsHtml += curSub[0];
		topPostsHtml += "</font></td><td><font size=1 color=#000000 face=verdana>";
		topPostsHtml += person.numPosts;
		topPostsHtml += "</font></td></tr>";
	}

	Thwonk.file.write(StatsTopPostsHtml, topPostsHtml);
}


function generateStatsTopBumps() {
	var i;
	var topBumpsHtml = "";

	var topBumpsList = eval(Thwonk.file.read(TopBumpsStatsSer));
	topBumpsList.sort(sortByTopBumps);
	topBumpsList.reverse();

	for(i = 0; i < topBumpsList.length; i++) {
		person = new Object_Subscriber(topBumpsList[i].userId);
		topBumpsHtml += "<tr align=left><td><font size=1 color=#000000 face=verdana>";
		curSub = person.userId.split("@", 2);
		topBumpsHtml += curSub[0];
		topBumpsHtml += "</font></td><td><font size=1 color=#000000 face=verdana>";
		topBumpsHtml += person.numBumps;
		topBumpsHtml += "</font></td></tr>";
	}

	Thwonk.file.write(StatsTopBumpsHtml, topBumpsHtml);
}



/*
 * Construct message to user telling them they have been bumped
*/
function genBumpedMsg() {
	var bumpedMsg = Thwonk.file.read(BumpedMsgPath_P1);

	if(Math.floor(Math.random() * 2) == 0) {
		bumpedMsg += "\\nJonah & Mike\\n";
	} else {
		bumpedMsg += "\\nMike & Jonah\\n";
	}

	bumpedMsg += Thwonk.file.read(BumpedMsgPath_P2);
	bumpedMsg += Thwonk.file.read(TagLinePath);

	return bumpedMsg;
}


/*
 * Construct message to user telling them they have subscribed to bumplist
*/
function genSignupMsg() {
	var signupMsg = Thwonk.file.read(SignupMsgPath_P1);

	if(Math.floor(Math.random() * 2) == 0) {
		signupMsg += "\\nMike & Jonah\\n";
	} else {
		signupMsg += "\\nJonah & Mike\\n";
	}

	signupMsg += Thwonk.file.read(SignupMsgPath_P2);
	signupMsg += Thwonk.file.read(TagLinePath);

	return signupMsg;
}


/*
 * Figure out who has the most time and insert them in the top time list if they
 * deserve it
*/
function checkAndAddTopTime(person) {
	var topTimeList = eval(Thwonk.file.read(TopTimeStatsSer));
	var diff = -1;
	var replace = -1;
	var ignore = -1;

	for(var i = 0; i < topTimeList.length; i++) {
		if(topTimeList[i].userId != person.userId) {
			if(person.totalTime > topTimeList[i].totalTime) {
				newDiff = person.totalTime - topTimeList[i].totalTime;

				if(diff == -1 || newDiff < diff) {
					diff = newDiff;
					replace = i;
				}
			}
		} else {
			ignore = 0;
		}
	}

	if(replace != -1 && ignore != 0) {
		topTimeList[replace] = person;
		Thwonk.file.write(TopTimeStatsSer, topTimeList.toSource());
	}
}


/*
 * Figure out who has the most posts and insert them in the top posts list if they
 * deserve it
*/
function checkAndAddTopPosts(person) {
	var topPostsList = eval(Thwonk.file.read(TopPostsStatsSer));
	var diff = -1;
	var replace = -1;
	var ignore = -1;

	for(var i = 0; i < topPostsList.length; i++) {
		if(topPostsList[i].userId != person.userId) {
			if(person.numPosts > topPostsList[i].numPosts) {
				newDiff = person.numPosts - topPostsList[i].numPosts;

				if(diff == -1 || newDiff < diff) {
					diff = newDiff;
					replace = i;
				}
			}
		} else {
			ignore = 0;
		}
	}

	if(replace != -1 && ignore != 0) {
		topPostsList[replace] = person;
		Thwonk.file.write(TopPostsStatsSer, topPostsList.toSource());
	}
}


/*
 * Figure out who has the most bumps and insert them in the top bumps list if they
 * deserve it
*/
function checkAndAddTopBumps(person) {
	var topBumpsList = eval(Thwonk.file.read(TopBumpsStatsSer));
	var diff = -1;
	var replace = -1;
	var ignore = -1;

	for(var i = 0; i < topBumpsList.length; i++) {
		if(topBumpsList[i].userId != person.userId) {
			if(person.numBumps > topBumpsList[i].numBumps) {
				newDiff = person.numBumps - topBumpsList[i].numBumps;

				if(diff == -1 || newDiff < diff) {
					diff = newDiff;
					replace = i;
				}
			}
		} else {
			ignore = 0;
		}
	}

	if(replace != -1 && ignore != 0) {
		topBumpsList[replace] = person;
		Thwonk.file.write(TopBumpsStatsSer, topBumpsList.toSource());
	}
}


/*
 * Subscribe a person to bumplist and bump off an existing one
*/
function subscribePerson(mail) {
	var subscribers = eval(Thwonk.file.read(SubscriberListPath));

	var fromEntry = ParseMail.getHeaderEntry("From", mail);
	var fromEnd = fromEntry.indexOf(" ");
	from = fromEntry.slice(0, fromEnd);
	from = from.toLowerCase();

	var subIds = [];

	for(var i = 0; i < subscribers.length; i++) {
		subIds[i] = subscribers[i].userId;
	}

	posFrom = subIds.indexOf(from);

	if(posFrom == -1) {
		var bumpedCount = eval(Thwonk.file.read(BumpCountPath));
		bumpedCount = bumpedCount + 1;
		Thwonk.file.write(BumpCountPath, bumpedCount.toSource());

		/* Bump a person */
		var unsub = subscribers.shift();
		unsub.numBumps = unsub.numBumps + 1;		// Record number of times person bumped
		unsub.totalTime = new Date().getTime() - Date.parse(unsub.subscribeTime) + unsub.totalTime;
		Thwonk.file.write(SubscribersStatPath + unsub.userId, unsub.toSource());

		checkAndAddTopBumps(unsub);
		checkAndAddTopTime(unsub);

		/* Subscribe new person */
		var subEntry = new Object_Subscriber(from);

		if(subEntry.numSubscribes != 0) {		// Has person previously subscribed?
			var ResubCount = eval(Thwonk.file.read(ResubCountPath));
			ResubCount = ResubCount + 1;		// Record total number of resubscribes
			Thwonk.file.write(ResubCountPath, ResubCount.toSource());
		}

		var curDate = new Date();			
		subEntry.subscribeTime = curDate.toLocaleString();

		subEntry.numSubscribes = subEntry.numSubscribes + 1;	// Number of subscribes
		subscribers.push(subEntry);
		Thwonk.file.write(SubscribersStatPath + subEntry.userId, subEntry.toSource());

		Thwonk.file.write(SubscriberListPath, subscribers.toSource());

		checkAndAddTopTime(subEntry);

		/* Tell subscriber and bumped what has happened */
		Thwonk.message.sendMember(1, unsub.userId, "[BumpList] BumpList Bumped", genBumpedMsg());
		Thwonk.message.sendMember(3, from, "[BumpList] BumpList Subscription", genSignupMsg());
	} else {
		Thwonk.message.sendMember(3, from, "[BumpList] Already Subscribed", genSignupMsg());
	}
}


/*
 * When a message is sent to the list send it out to subscribers
 * if a subscriber sent it
*/
function messageToSubscribers(mail) {
	var subscribers = eval(Thwonk.file.read(SubscriberListPath));

	var fromEntry = ParseMail.getHeaderEntry("From", mail);
	var fromEnd = fromEntry.indexOf(" ");
	from = fromEntry.slice(0, fromEnd);
	from = from.toLowerCase();

	var subIds = [];

	for(var i = 0; i < subscribers.length; i++) {
		subIds[i] = subscribers[i].userId;
	}

	posFrom = subIds.indexOf(from);

	if(from != BumplistAddress && posFrom != -1) {
		checkAndAddTopPosts(new Object_Subscriber(from));

		var subject = ParseMail.getHeaderEntry("Subject:", mail);
		var body = ParseMail.getBody(mail);

		/* Manage the list of current top subjects */
		var subPerson = new Object_Subscriber(from);
		var curDate = new Date();
		var subObject = new Object_SubjectListEntry(subject, curDate.toLocaleString(), figureTotalTime(subPerson));

		var subjectsList = eval(Thwonk.file.read(SubjectsListPath));
		subjectsList.shift();
		subjectsList.push(subObject);
		Thwonk.file.write(SubjectsListPath, subjectsList.toSource());

		/* Record that another message has been processed */
		var messageCount = eval(Thwonk.file.read(MessageCountPath));
		messageCount = messageCount + 1;
		Thwonk.file.write(MessageCountPath, messageCount.toSource());

		/* Record which subscriber sent a message */
		subscribers[posFrom].numPosts = subscribers[posFrom].numPosts + 1;

		/* Sent out message */
		for(var i = 0; i < subIds.length; i++) {
			Thwonk.print("Message to: ", subIds[i]);
			Thwonk.message.sendMember(3, subIds[i], "[BumpList] " + subject, body);
			
			/* Record that member received a message */
			subscribers[i].numMessagesReceived = subscribers[i].numMessagesReceived + 1;
			Thwonk.file.write(SubscribersStatPath + subIds[i], subscribers[i].toSource());
		}

		/* Update information stored in stat file for current active subscribers */
		Thwonk.file.write(SubscriberListPath, subscribers.toSource());
	}
}' where id = 1;
