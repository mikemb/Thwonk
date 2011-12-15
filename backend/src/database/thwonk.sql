/*
 * Author: Mike Bennett (mike@thwonk.com)
 *
 * Purpose: Database design with starting data for Thwonk
 *
 * Note: Store procedures aren't used, logic is kept on the client
 *       side. Apparently keeps DB resource usage down and should
 *	 make easier to implement with different DBs
 *
 * Note 2: When looking at values that fields in the database can
 *	 be you should use dbchatter.h as the definitive reference
*/


/*
 * User accounts
*/
CREATE TABLE user (
	id		BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,	# Unique id for user
	username	VARCHAR(100) NOT NULL,		# User username
	pword		CHAR(41) NOT NULL,		# Their password
	lastLogin	DATETIME,			# Last login date
	accType		INT UNSIGNED NOT NULL,		# Type of account, 100 = admin, 1000 = normal user,
							#  1 = needs to be activated, 200 = no login,
							#  300 = special unkown user
	emailId		BIGINT UNSIGNED,		# Main contact email address, see filter_user id
	
	UNIQUE(username),
	INDEX(username)
) type=InnoDB;


/*
 * User account activation
*/
CREATE TABLE user_activation (
	id		BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,	# Unique id for user_activation
	actString	VARCHAR(100) NOT NULL,		# Activation string
	regDate		DATETIME,			# Activation required since
	userId		BIGINT UNSIGNED NOT NULL,	# User Id to be activated

	FOREIGN KEY(userId) REFERENCES user(id),

	UNIQUE(actString),
	UNIQUE(userId),
	INDEX(actString)
) type=InnoDB;


/*
 * Void (e.g. email lists, etc)
*/
CREATE TABLE void (
	id		BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,	# Unique id for void 
	name		VARCHAR(500) NOT NULL,		# Void name
	blurb		VARCHAR(1000)			# Blurb about the void
) type=InnoDB;


/*
 * Each user can be a member of multiple voids, but doesn't have to be (depending on void type)
*/
CREATE TABLE void_membership (
	id		BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,	# Unique id for void_membership
	userId		BIGINT UNSIGNED NOT NULL,	# Id of user signed up to the void
	voidId		BIGINT UNSIGNED NOT NULL,	# Void id the user is part of
	privilege	BIGINT UNSIGNED NOT NULL,	# What privilege the user has to manage the void (Note:
							#  this is here because we want to always have at least
							#  one user responsible for each void)
	FOREIGN KEY(userId) REFERENCES user(id),
	FOREIGN KEY(voidId) REFERENCES void(id)
) type=InnoDB;


/*
 * Control who can submit content
*/
CREATE TABLE filter_user (
	id		BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,	# Unique id for user filter
	identifier	VARCHAR(1000) NOT NULL,		# Identifier, e.g. email address
	filterType	INT UNSIGNED NOT NULL,		# Email (1) (later on IM, etc)
	status		INT UNSIGNED NOT NULL,		# Active(1), inactive, suspended, banned, blocked
	userId		BIGINT UNSIGNED NOT NULL,	# User id associated with this filter

	FOREIGN KEY(userId) REFERENCES user(id),

	INDEX(identifier),
	INDEX(filterType),
	INDEX(userId)
) type=InnoDB;


/*
 * Control how content can be submitted to a void
*/
CREATE TABLE filter_void (
	id		BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,	# Unique id for void filter
	identifier	VARCHAR(1000) NOT NULL,		# Identifier (target name for void)
	filterType	INT UNSIGNED NOT NULL,		# Email (1) (later on IM, etc)
	status		INT UNSIGNED NOT NULL,		# Active(1), inactive, suspended, banned, blocked
	accessRights	INT UNSIGNED NOT NULL,		# How this filter enables anyone to send content
							#  to the void (1 = public to anyone, 2 = public to
							#  thwonk members, 3 = public to members of this void)
	voidId		BIGINT UNSIGNED NOT NULL,	# Void id associated with this filter

	FOREIGN KEY(voidId) REFERENCES void(id),

	INDEX(identifier),
	INDEX(filterType),
	INDEX(voidId)
) type=InnoDB;


/*
 * Messages received and sending
*/
CREATE TABLE message (
	id		BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,	# Unique id for message 
	userId		BIGINT UNSIGNED NOT NULL,	# User id of person who sent/is receiving this message
	filterVoidId	BIGINT UNSIGNED,		# Void filter associated with this message
	filterUserId	BIGINT UNSIGNED,		# User filter associated with this message

	messageType	INT UNSIGNED NOT NULL,		# 1 = unknown,
							#
							# 2 = everything (used by filter),
							#
							# 3 = email in
							#	(filterUserId = From,
							#	filterVoidId = Dest Void)
							#
							# For 4 - 11 message_protocol_mail has pointers to correct
							#  to, from and reply to field values
							#
							# 4 = email out, from user, to void member, reply to user
							# 5 = email out, from user, to void member, reply to void
							# 6 = email out, from void, to void member, reply to user
							# 7 = email out, from void, to void member, reply to void
							# 8 = email out, from user, to all members, reply to user
							# 9 = email out, from user, to all members, reply to void 
							# 10 = email out, from void, to all members, reply to user 
							# 11 = email out, from void, to all members, reply to void 

	messageState	INT UNSIGNED NOT NULL,		# State of the message, 1 = just in, 2 = getting processed
							#  by rule runner, 3 = done running rules
	processDate	DATETIME,			# Date & Time this message was last acted upon
	rawContent	BLOB NOT NULL,			# Raw content of the message

	FOREIGN KEY(userId) REFERENCES user(id),
	FOREIGN KEY(filterVoidId) REFERENCES filter_void(id),
	FOREIGN KEY(filterUserId) REFERENCES filter_user(id)
) type=InnoDB;


/*
 * Store Mail protocol specific info. Needed for creating outgoing mail headers
*/
CREATE TABLE message_protocol_mail (
	id			BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,	# Unique id for row
	messageId		BIGINT UNSIGNED,	# Message this header info is associated with
	toFilterUserId		BIGINT UNSIGNED,
	toFilterVoidId		BIGINT UNSIGNED,
	fromFilterUserId	BIGINT UNSIGNED,
	fromFilterVoidId	BIGINT UNSIGNED,
	replytoFilterUserId	BIGINT UNSIGNED,
	replytoFilterVoidId	BIGINT UNSIGNED,

	FOREIGN KEY(messageId) REFERENCES message(id),

	INDEX(messageId)
) type=InnoDB;


/*
 * Queue for managing messages
*/
CREATE TABLE message_queue (
	id		BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,	# Unique id for message
	messageId	BIGINT UNSIGNED NOT NULL,	# Message id for this queue entry
	messageType	INT UNSIGNED NOT NULL,		# See message.messageType for details
	queueState	INT UNSIGNED,			# State of this queue entry
	userId		BIGINT UNSIGNED,		# User (sender) id (if any) associated with this message
							#
							# If messageType = 
							#
							#  DBVAL_message_queue_messageType_EMAILOUT then userId is
							#  the outgoing user this message is destinated for
	voidId		BIGINT UNSIGNED NOT NULL,	# Void id this message is destinated for or from
	track		INT,				# Can be used for maintaining different priority queues
	processDate	DATETIME,			# Date & Time this queue item was last acted upon

	FOREIGN KEY(messageId) REFERENCES message(id),
	FOREIGN KEY(userId) REFERENCES user(id),
	FOREIGN KEY(voidId) REFERENCES void(id),

	INDEX(voidId),
	INDEX(queueState),
	INDEX(messageType),
	INDEX(track)
) type=InnoDB;


/*
 * Scripts / programs to implement rules, etc
*/
CREATE TABLE logic (
	id		BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,	# Unique id for logic
	name		VARCHAR(500) NOT NULL,		# Logic unit name
	blurb		VARCHAR(1000),			# Blurb about the logic

	version		INT UNSIGNED,			# Version number of this script
	editDate	DATETIME,			# Date & Time this script was last edited
	language	INT UNSIGNED,			# Language logic is written in (1 = Javascript)
	logic		VARCHAR(100000),		# Logic (script)
	logicCache	VARCHAR(100000)			# Compiled version of logic (if possible)
) type=InnoDB;


/*
 * Manage rights to logics, edit right, use right, etc
*/
CREATE TABLE logic_rights (
	id		BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,	# Unique id for logic
	logicId		BIGINT UNSIGNED NOT NULL,	# Id of logic this right applies to
	rightType	INT UNSIGNED NOT NULL,		# What kind of right is it, a user (1) or a void (2)?
	userId		BIGINT UNSIGNED,		# User associated with this right
	voidId		BIGINT UNSIGNED,		# Void associated with this right
	filterVoidId	BIGINT UNSIGNED,		# Void filter associated with this right
	useRight	INT UNSIGNED,			# 0 = allowed to use/read
	editRight	INT UNSIGNED,			# 0 = allowed to edit
	delRight	INT UNSIGNED,			# 0 = allowed to delete
	viewRight	INT UNSIGNED,			# 0 = allowed to view
	rightRight	INT UNSIGNED,			# 0 = allowed to change rights

	FOREIGN KEY(logicId) REFERENCES logic(id),
	FOREIGN KEY(filterVoidId) REFERENCES filter_void(id)
) type=InnoDB;


/*
 * Support virtual files by storing them in the database
*/
CREATE TABLE vfile (
	id		BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,	# Unique id for file
	fileType	INT UNSIGNED,			# File type (reserved for now)
	editDate	DATETIME,			# Date & Time the file was last edited
	name		VARCHAR(500) NOT NULL,		# File name including full path
	content		BLOB				# File content

) type=InnoDB;


/*
 * Manage the rights to a vfile
*/
CREATE TABLE vfile_rights (
	id		BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,	# Unique id for file rights
	vfileId		BIGINT UNSIGNED NOT NULL,	# Id of vfile this set of rights applies to
	userId		BIGINT UNSIGNED,		# If present then user id these rights apply to
	voidId		BIGINT UNSIGNED,		# if present then void id these rights apply to

	useRight	INT UNSIGNED,			# 0 = allowed to execute/use
	editRight	INT UNSIGNED,			# 0 = allowed to edit
	delRight	INT UNSIGNED,			# 0 = allowed to delete
	viewRight	INT UNSIGNED,			# 0 = allowed to view
	rightRight	INT UNSIGNED,			# 0 = allowed to change rights

	FOREIGN KEY(vfileId) REFERENCES vfile(id)
) type=InnoDB;


/*
 * Populate DB with basic data
*/
INSERT INTO user (id, username, pword, lastLogin, accType, emailId) VALUES (1, 'admin', PASSWORD('changepassword'), now(), 100, 1);
INSERT INTO user (id, username, pword, lastLogin, accType, emailId) VALUES (2, 'THWONK_NOLOGIN_USER', PASSWORD('nologin'), now(), 200, 1);
INSERT INTO user (id, username, pword, lastLogin, accType, emailId) VALUES (5, 'THWONK_UNKNOWN_USER', PASSWORD('nologin'), now(), 300, 1);
INSERT INTO user (id, username, pword, lastLogin, accType, emailId) VALUES (3, 'mike', PASSWORD('changepassword'), now(), 1000, 2);
INSERT INTO user (id, username, pword, lastLogin, accType, emailId) VALUES (4, 'mikeb', PASSWORD('changepassword'), now(), 1000, 3);
INSERT INTO filter_user (id, identifier, filterType, status, userId) VALUES (1, 'mike@thwonk.com', 1, 1, 1);
INSERT INTO filter_user (id, identifier, filterType, status, userId) VALUES (2, 'mike@thwonk.com', 1, 1, 3);
INSERT INTO filter_user (id, identifier, filterType, status, userId) VALUES (3, 'mikeb@thwonk.com', 1, 1, 4);
INSERT INTO void (id, name, blurb) VALUES (1, 'thwonk', '..and then there was thwonk!');
INSERT INTO void (id, name, blurb) VALUES (3, 'thwonk3', '..and then there was thwonk!');
INSERT INTO void (id, name, blurb) VALUES (4, 'thwonk4', '..and then there was thwonk!');
INSERT INTO void (id, name, blurb) VALUES (5, 'thwonk5', '..and then there was thwonk!');
INSERT INTO void (id, name, blurb) VALUES (6, 'thwonk6', '..and then there was thwonk!');
INSERT INTO void (id, name, blurb) VALUES (7, 'thwonk7', '..and then there was thwonk!');
INSERT INTO void (id, name, blurb) VALUES (8, 'thwonk8', '..and then there was thwonk!');
INSERT INTO void (id, name, blurb) VALUES (9, 'thwonk9', '..and then there was thwonk!');
INSERT INTO void (id, name, blurb) VALUES (10, 'thwonk10', '..and then there was thwonk!');
INSERT INTO void (id, name, blurb) VALUES (11, 'thwonk11', '..and then there was thwonk!');
INSERT INTO void (id, name, blurb) VALUES (12, 'thwonk12', '..and then there was thwonk!');
INSERT INTO void (id, name, blurb) VALUES (13, 'thwonk13', '..and then there was thwonk!');
INSERT INTO void (id, name, blurb) VALUES (14, 'thwonk14', '..and then there was thwonk!');
INSERT INTO void (id, name, blurb) VALUES (15, 'thwonk15', '..and then there was thwonk!');
INSERT INTO void (id, name, blurb) VALUES (16, 'thwonk16', '..and then there was thwonk!');
INSERT INTO void (id, name, blurb) VALUES (17, 'thwonk17', '..and then there was thwonk!');
INSERT INTO void (id, name, blurb) VALUES (18, 'thwonk18', '..and then there was thwonk!');
INSERT INTO void (id, name, blurb) VALUES (2, 'test', 'test void');
INSERT INTO void (id, name, blurb) VALUES (19, 'play', '..and then there was thwonk!');
INSERT INTO void_membership (userId, voidId, privilege) VALUES (1, 1, 1);
INSERT INTO void_membership (userId, voidId, privilege) VALUES (3, 1, 1);
INSERT INTO void_membership (userId, voidId, privilege) VALUES (3, 2, 1);
INSERT INTO void_membership (userId, voidId, privilege) VALUES (3, 3, 1);
INSERT INTO void_membership (userId, voidId, privilege) VALUES (3, 4, 1);
INSERT INTO void_membership (userId, voidId, privilege) VALUES (3, 5, 1);
INSERT INTO void_membership (userId, voidId, privilege) VALUES (3, 6, 1);
INSERT INTO void_membership (userId, voidId, privilege) VALUES (3, 7, 1);
INSERT INTO void_membership (userId, voidId, privilege) VALUES (3, 8, 1);
INSERT INTO void_membership (userId, voidId, privilege) VALUES (3, 9, 1);
INSERT INTO void_membership (userId, voidId, privilege) VALUES (3, 10, 1);
INSERT INTO void_membership (userId, voidId, privilege) VALUES (3, 11, 1);
INSERT INTO void_membership (userId, voidId, privilege) VALUES (3, 12, 1);
INSERT INTO void_membership (userId, voidId, privilege) VALUES (3, 13, 1);
INSERT INTO void_membership (userId, voidId, privilege) VALUES (3, 14, 1);
INSERT INTO void_membership (userId, voidId, privilege) VALUES (3, 15, 1);
INSERT INTO void_membership (userId, voidId, privilege) VALUES (3, 16, 1);
INSERT INTO void_membership (userId, voidId, privilege) VALUES (3, 17, 1);
INSERT INTO void_membership (userId, voidId, privilege) VALUES (3, 18, 1);
INSERT INTO void_membership (userId, voidId, privilege) VALUES (3, 19, 1);
INSERT INTO void_membership (userId, voidId, privilege) VALUES (4, 19, 1);
INSERT INTO filter_void (id, identifier, filterType, status, accessRights, voidId) VALUES  (1, 'thwonk1@thwonk.com', 1, 1, 1, 1);
INSERT INTO filter_void (id, identifier, filterType, status, accessRights, voidId) VALUES  (3, 'thwonk3@thwonk.com', 1, 1, 1, 3);
INSERT INTO filter_void (id, identifier, filterType, status, accessRights, voidId) VALUES  (4, 'thwonk4@thwonk.com', 1, 1, 1, 4);
INSERT INTO filter_void (id, identifier, filterType, status, accessRights, voidId) VALUES  (5, 'thwonk5@thwonk.com', 1, 1, 1, 5);
INSERT INTO filter_void (id, identifier, filterType, status, accessRights, voidId) VALUES  (6, 'thwonk6@thwonk.com', 1, 1, 1, 6);
INSERT INTO filter_void (id, identifier, filterType, status, accessRights, voidId) VALUES  (7, 'thwonk7@thwonk.com', 1, 1, 1, 7);
INSERT INTO filter_void (id, identifier, filterType, status, accessRights, voidId) VALUES  (8, 'thwonk8@thwonk.com', 1, 1, 1, 8);
INSERT INTO filter_void (id, identifier, filterType, status, accessRights, voidId) VALUES  (9, 'thwonk9@thwonk.com', 1, 1, 1, 9);
INSERT INTO filter_void (id, identifier, filterType, status, accessRights, voidId) VALUES  (10, 'thwonk10@thwonk.com', 1, 1, 1, 10);
INSERT INTO filter_void (id, identifier, filterType, status, accessRights, voidId) VALUES  (11, 'thwonk11@thwonk.com', 1, 1, 1, 11);
INSERT INTO filter_void (id, identifier, filterType, status, accessRights, voidId) VALUES  (12, 'thwonk12@thwonk.com', 1, 1, 1, 12);
INSERT INTO filter_void (id, identifier, filterType, status, accessRights, voidId) VALUES  (13, 'thwonk13@thwonk.com', 1, 1, 1, 13);
INSERT INTO filter_void (id, identifier, filterType, status, accessRights, voidId) VALUES  (14, 'thwonk14@thwonk.com', 1, 1, 1, 14);
INSERT INTO filter_void (id, identifier, filterType, status, accessRights, voidId) VALUES  (15, 'thwonk15@thwonk.com', 1, 1, 1, 15);
INSERT INTO filter_void (id, identifier, filterType, status, accessRights, voidId) VALUES  (16, 'thwonk16@thwonk.com', 1, 1, 1, 16);
INSERT INTO filter_void (id, identifier, filterType, status, accessRights, voidId) VALUES  (17, 'thwonk17@thwonk.com', 1, 1, 1, 17);
INSERT INTO filter_void (id, identifier, filterType, status, accessRights, voidId) VALUES  (18, 'thwonk18@thwonk.com', 1, 1, 1, 18);
INSERT INTO filter_void (id, identifier, filterType, status, accessRights, voidId) VALUES  (19, 'play@thwonk.com', 1, 1, 1, 19);
INSERT INTO filter_void (id, identifier, filterType, status, accessRights, voidId) VALUES  (2, 'test@thwonk.com', 1, 1, 3, 2);
INSERT INTO logic (id, name, blurb, version, editDate, language, logic) VALUES (1, "Test Script", "Basic test script", 1, now(), 1, "Thwonk.print(\"From database\");Thwonk.message.sendMember(1, \"mike\", \"This is subject\", \"This is body\");Thwonk.message.sendMember(1, \"admin\", \"This is subject\", \"This is body admin\");");
INSERT INTO logic (id, name, blurb, version, editDate, language, logic) VALUES (2, "Test Script 2", "Test script 2", 1, now(), 1, "Thwonk.message.sendMember(1, \"mike\", \"This is subject\", \"Body 2\");");
INSERT INTO logic (id, name, blurb, version, editDate, language, logic) VALUES (3, "Test Script 3", "Serialise and deserialise objects", 1, now(), 1, "var newFunction = eval('(' + Thwonk.file.read(\"/blah/object.file\") + ')'); Thwonk.print('moo is: ' + newFunction);");
INSERT INTO logic_rights (id, logicId, rightType, userId, voidId, filterVoidId, useRight, editRight, delRight, viewRight, rightRight) VALUES (1, 1, 2, 0, 1, 1, 1, 2, 2, 2, 2);
INSERT INTO logic_rights (id, logicId, rightType, userId, voidId, filterVoidId, useRight, editRight, delRight, viewRight, rightRight) VALUES (3, 1, 2, 0, 3, 3, 1, 2, 2, 2, 2);
INSERT INTO logic_rights (id, logicId, rightType, userId, voidId, filterVoidId, useRight, editRight, delRight, viewRight, rightRight) VALUES (4, 1, 2, 0, 4, 4, 1, 2, 2, 2, 2);
INSERT INTO logic_rights (id, logicId, rightType, userId, voidId, filterVoidId, useRight, editRight, delRight, viewRight, rightRight) VALUES (5, 1, 2, 0, 5, 5, 1, 2, 2, 2, 2);
INSERT INTO logic_rights (id, logicId, rightType, userId, voidId, filterVoidId, useRight, editRight, delRight, viewRight, rightRight) VALUES (6, 1, 2, 0, 6, 6, 1, 2, 2, 2, 2);
INSERT INTO logic_rights (id, logicId, rightType, userId, voidId, filterVoidId, useRight, editRight, delRight, viewRight, rightRight) VALUES (7, 1, 2, 0, 7, 7, 1, 2, 2, 2, 2);
INSERT INTO logic_rights (id, logicId, rightType, userId, voidId, filterVoidId, useRight, editRight, delRight, viewRight, rightRight) VALUES (8, 1, 2, 0, 8, 8, 1, 2, 2, 2, 2);
INSERT INTO logic_rights (id, logicId, rightType, userId, voidId, filterVoidId, useRight, editRight, delRight, viewRight, rightRight) VALUES (9, 1, 2, 0, 9, 9, 1, 2, 2, 2, 2);
INSERT INTO logic_rights (id, logicId, rightType, userId, voidId, filterVoidId, useRight, editRight, delRight, viewRight, rightRight) VALUES (10, 1, 2, 0, 10, 10, 1, 2, 2, 2, 2);
INSERT INTO logic_rights (id, logicId, rightType, userId, voidId, filterVoidId, useRight, editRight, delRight, viewRight, rightRight) VALUES (11, 1, 2, 0, 11, 11, 1, 2, 2, 2, 2);
INSERT INTO logic_rights (id, logicId, rightType, userId, voidId, filterVoidId, useRight, editRight, delRight, viewRight, rightRight) VALUES (12, 1, 2, 0, 12, 12, 1, 2, 2, 2, 2);
INSERT INTO logic_rights (id, logicId, rightType, userId, voidId, filterVoidId, useRight, editRight, delRight, viewRight, rightRight) VALUES (13, 1, 2, 0, 13, 13, 1, 2, 2, 2, 2);
INSERT INTO logic_rights (id, logicId, rightType, userId, voidId, filterVoidId, useRight, editRight, delRight, viewRight, rightRight) VALUES (14, 1, 2, 0, 14, 14, 1, 2, 2, 2, 2);
INSERT INTO logic_rights (id, logicId, rightType, userId, voidId, filterVoidId, useRight, editRight, delRight, viewRight, rightRight) VALUES (15, 1, 2, 0, 15, 15, 1, 2, 2, 2, 2);
INSERT INTO logic_rights (id, logicId, rightType, userId, voidId, filterVoidId, useRight, editRight, delRight, viewRight, rightRight) VALUES (16, 1, 2, 0, 16, 16, 1, 2, 2, 2, 2);
INSERT INTO logic_rights (id, logicId, rightType, userId, voidId, filterVoidId, useRight, editRight, delRight, viewRight, rightRight) VALUES (17, 1, 2, 0, 17, 17, 1, 2, 2, 2, 2);
INSERT INTO logic_rights (id, logicId, rightType, userId, voidId, filterVoidId, useRight, editRight, delRight, viewRight, rightRight) VALUES (18, 1, 2, 0, 18, 18, 1, 2, 2, 2, 2);
INSERT INTO logic_rights (id, logicId, rightType, userId, voidId, filterVoidId, useRight, editRight, delRight, viewRight, rightRight) VALUES (19, 2, 2, 0, 19, 19, 1, 2, 2, 2, 2);
