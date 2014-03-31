#!/usr/bin/env python2.7

import os;
import sys;
import socket;

import smtplib
import time

def smtp_test(host, port, user, passwd, touser):
    try:
        smtp = smtplib.SMTP()
        smtp.set_debuglevel(1)
        smtp.connect(host, port)
        smtp.login(user, passwd)
        msg = "From: %s\r\nTo: %s\r\n"%(user, touser)
        msg = msg + "Subject: this is a test mail\r\n\r\n" 
        msg = msg + "hello this is a test mail.\r\n" + time.ctime()
        smtp.sendmail(user, touser, msg)
        smtp.quit()
    except Exception as e:
        print "mailerror smtp_test() %s %s error:" % (host, user) 
        print e
#end

def smtpssl_test(host, port, user, passwd, touser):
    try:
        smtp_ssl = smtplib.SMTP_SSL()
        smtp_ssl.set_debuglevel(1)
        smtp_ssl.connect(host, port)
        smtp_ssl.login(user, passwd)
        msg = "From: %s\r\nTo: %s\r\n"%(user, touser)
        msg = msg + "Subject: this is a test mail\r\n\r\n" 
        msg = msg + "hello this is a test mail.\r\n" + time.ctime()
        smtp_ssl.sendmail(user, touser, msg)
        smtp_ssl.quit()
    except Exception as e:
        print "mailerror smtpssl_test() %s %s error:" % (host, user)
        print e
#end

def smtptls_test(host, port, user, passwd, touser):
    try:
        smtp_tls = smtplib.SMTP()
        smtp_tls.set_debuglevel(1)
        smtp_tls.connect(host, port)
        smtp_tls.starttls()
        smtp_tls.login(user, passwd)
        msg = "From: %s\r\nTo: %s\r\n"%(user, touser)
        msg = msg + "Subject: this is a test mail\r\n\r\n" 
        msg = msg + "hello this is a test mail.\r\n" + time.ctime()
        smtp_tls.sendmail(user, touser, msg)
        smtp_tls.quit()
    except Exception as e:
        print "mailerror smtptls_test() %s %s error:" % (host, user)
        print e
#end

#########################################################################

import poplib
import time

def pop_test(host, port, user, passwd):
    try:
        pop = poplib.POP3(host, port)
        pop.set_debuglevel(2)
        pop.user(user)
        pop.pass_(passwd)
        status = pop.stat()
        lists = pop.list()
        mailstr = pop.retr(1)
        pop.quit()
    except Exception as e:
        print "mailerror pop_test() %s %s error:" % (host, user)
        print e
#end

def popssl_test(host, port, user, passwd):
    try:
        pop_ssl = poplib.POP3_SSL(host, port)
        pop_ssl.set_debuglevel(2)
        pop_ssl.user(user)
        pop_ssl.pass_(passwd)
        status = pop_ssl.stat()
        lists = pop_ssl.list()
        mailstr = pop_ssl.retr(1)
        pop_ssl.quit()
    except Exception as e:
        print "mailerror pop_test() %s %s error:" % (host, user)
        print e
#end

##########################################################

import imaplib, email

def imap_test(host, port, user, passwd):
    try:
        imap = imaplib.IMAP4(host, port)  
        imap.debug = 4
        imap.login(user, passwd)
        list = imap.list()
        select = imap.select("INBOX")
        head = imap.fetch(1, "rfc822.header")
        imap.logout()
    except Exception as e:
        print "mailerror imap_test() %s %s error:" % (host, user)
        print e
#end

def imapssl_test(host, port, user, passwd):
    try:
        imap_ssl = imaplib.IMAP4_SSL(host, port)  
        imap_ssl.debug = 4
        imap_ssl.login(user, passwd)
        list = imap_ssl.list()
        select = imap_ssl.select("INBOX")
        head = imap_ssl.fetch(1, "rfc822.header")
        imap_ssl.logout()
    except Exception as e:
        print "mailerror imapssl_test() %s %s error:" % (host, user)
        print e
#end

if __name__ == "__main__":
    """ test mail """

    socket.setdefaulttimeout(3)

    smtp_test("xxx", 25, "xxx", "xxx", "xxx");

    
#end
