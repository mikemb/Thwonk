/*
 * Provide a ParseMail object that parses email messages
 *
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Methods:
 *	- ParseMail.getHeaderEntry(entry, mail)
 *	- ParseMail.getBody(mail)
*/
function InternalParseMail() {

	this.getHeaderEntry = function (entry, mail) {
		var subStart = mail.indexOf(entry);
		var subEnd = mail.indexOf("\\n", subStart);

		subLine = mail.slice(subStart, subEnd);

		subStart = subLine.indexOf(" ");

		var subject = subLine.slice(subStart + 1);

		subject = String.replace(subject, /;/g, "&#59;");
		subject = String.replace(subject, /</g, "&lt;");
		subject = String.replace(subject, />/g, "&gt;");

		return subject;
	};

	this.getBody = function (mail) {
		var bodyStart = mail.indexOf("\\n\\n");

		body = mail.slice(bodyStart + 2);

		return body;
	};
}

var ParseMail = new InternalParseMail();
