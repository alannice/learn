#!/usr/bin/env python2.7

import os;
import sys;

import smtplib
import time

def smtp_test(host, port, user, passwd, touser):
    smtp = smtplib.SMTP()
    smtp.set_debuglevel(1)
    smtp.connect("smtp.sina.com", "25")
    smtp.login("zhch_todo@sina.com", "yuanzc")
    smtp.sendmail("zhch_todo@sina.com", "zhch_todo@sina.cn", 
            "From: zhch_todo@sina.com\r\nTo: zhch_todo@sina.cn\r\nSubject: this is a test mail\r\n\r\n"+
            "hello this is a test mail.\r\n"+time.ctime())
    smtp.quit()
#end

def smtpssl_test(host, port, user, passwd, touser):
    smtp_ssl = smtplib.SMTP_SSL()
    smtp_ssl.set_debuglevel(1)
    smtp_ssl.connect("smtp.sina.com", "465")
    smtp_ssl.login("zhch_todo@sina.com", "yuanzc")
    smtp_ssl.sendmail("zhch_todo@sina.com", "zhch_todo@sina.cn", 
            "From: zhch_todo@sina.com\r\nTo: zhch_todo@sina.cn\r\nSubject: this is a test mail\r\n\r\n"+
            "hello this is a test mail.\r\n"+time.ctime()+"\r\n")
    smtp_ssl.quit()
#end

def smtptls_test(host, port, user, passwd, touser):
    smtp_tls = smtplib.SMTP()
    smtp_tls.set_debuglevel(1)
    smtp_tls.connect("smtp.sina.com", "25")
    smtp_tls.starttls()
    smtp_tls.login("zhch_todo@sina.com", "yuanzc")
    smtp_tls.sendmail("zhch_todo@sina.com", "zhch_todo@sina.cn", 
            "From: zhch_todo@sina.com\r\nTo: zhch_todo@sina.cn\r\nSubject: this is a test mail\r\n\r\n"+
            "hello this is a test mail.\r\n"+time.ctime())
    smtp_tls.quit()
#end

#########################################################################

import poplib
import time

def pop_test(host, port, user, passwd):
    pop = poplib.POP3("pop3.sina.com", 110)
    pop.user("zhch_todo@sina.com")
    pop.pass_("yuanzc")
    status = pop.stat()
    print status
    lists = pop.list()
    print lists
    for item in lists[1]:
        num, octets = item.split(' ')
        print "msg: "+num+", bytes: "+octets

    mailstr = pop.retr(3)

    recvmail = open("./aa", "w")
    for line in mailstr[1]:
        recvmail.write(line+"\r\n")
    recvmail.close()

    pop.quit()
#end

def popssl_test(host, port, user, passwd):
    pop_ssl = poplib.POP3_SSL("pop3.sina.com", 995)
    pop_ssl.user("zhch_todo@sina.com")
    pop_ssl.pass_("yuanzc")
    status = pop_ssl.stat()
    print status
    lists = pop_ssl.list()
    print lists
    for item in lists[1]:
        num, octets = item.split(' ')
        print "msg: "+num+", bytes: "+octets

    mailstr = pop_ssl.retr(3)

    recvmail = open("./aa", "w")
    for line in mailstr[1]:
        recvmail.write(line+"\r\n")
    recvmail.close()

    pop_ssl.quit()
#end

##########################################################

import imaplib, email

def imap_test(host, port, user, passwd):
    #imap = imaplib.IMAP4('imap.sina.com')  
    imap = imaplib.IMAP4_SSL('imap.sina.com')  
    imap.login("zhangcheng_todo@sina.com", "yuanzc")

    list = imap.list()
    print list

    select = imap.select("INBOX")
    print select

    #head = imap.fetch(1, "rfc822.header")
    head = imap.fetch(1, "(body[header.fields (Subject)])")
    #head = imap.fetch(3, "rfc822")
    print head

    typ,msg_data = imap.fetch('1','(BODY.PEEK[HEADER])')  
    for response_part in msg_data:  
        if isinstance(response_part,tuple):  
            msg = email.message_from_string(response_part[1])  
            for header in ['subject','to','from']:  
                print '%-8s: %s'%(header.upper(),msg[header])  
                h = email.Header.Header(msg[header])
                dh = email.Header.decode_header(h) 
                print dh

    imap.logout()
#end

def imapssl_test(host, port, user, passwd):
    imap_ssl = imaplib.IMAP4_SSL('imap.sina.com')  
    imap_ssl.login("zhangcheng_todo@sina.com", "yuanzc")

    list = imap_ssl.list()
    print list

    select = imap_ssl.select("INBOX")
    print select

    #head = imap_ssl.fetch(1, "rfc822.header")
    head = imap_ssl.fetch(1, "(body[header.fields (Subject)])")
    #head = imap.fetch(3, "rfc822")
    print head

    typ,msg_data = imap_ssl.fetch('1','(BODY.PEEK[HEADER])')  
    for response_part in msg_data:  
        if isinstance(response_part,tuple):  
            msg = email.message_from_string(response_part[1])  
            for header in ['subject','to','from']:  
                print '%-8s: %s'%(header.upper(),msg[header])  
                h = email.Header.Header(msg[header])
                dh = email.Header.decode_header(h) 
                print dh

    imap_ssl.logout()
#end

if __name__ == "__main__":
    """ test mail """

    if len(sys.argv) != 6:
        print "Usage: %s ip port smtp|smtps|smtptls|pop|pops|imap|imaps user passwd" % sys.argv[0]
        sys.exit(0)

         
