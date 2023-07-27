from flask import Flask, jsonify, make_response
import datetime

app = Flask(__name__)

@app.route('/')
def example():
    # here to change headers
    headers = {
        'Content-Type': 'application/json',
        'Expires': 'Wed, 21 Oct 2025 07:28:00 GMT',
        'Cache-Control': 'max-age=360000',
    }
    # test cache-control: no-cache 
    # cached, but requires re-validation
    # in cache, requires validation
    headers_1 = {
        'Content-Type': 'application/json',
        'Date': 'Wed, 21 Oct 2014 07:28:00 GMT',
        'Expires': 'Wed, 21 Oct 2025 07:28:00 GMT',
        'Cache-Control': 'no-cache',
        'Host': '67.159.88.155:5000',
        'ETag': '10000',
    }
    # test Expires header(expired) w/ cache-control:must-revalidate
    # cached, but requires re-validation
    # in cache, requires validation
    headers_2 = {
        'Content-Type': 'application/json',
        'Date': 'Wed, 21 Oct 2014 07:28:00 GMT',
        'Expires': 'Wed, 21 Oct 2015 07:28:00 GMT',
        'Cache-Control': 'must-revalidate',
        'Host': '67.159.88.155:5000',
        'ETag': '10000',
    }
    # test Expires header(not expired) w/ cache-control:must-revalidate
    # cached, expires at EXPIRES
    # if not expires - in cache, valid 
    # if expires - in cache, requires validation 
    # https://edstem.org/us/courses/32653/discussion/2675051
    headers_3 = {
        'Content-Type': 'application/json',
        'Date': 'Tue, 21 Oct 2014 07:28:00 GMT',
        'Expires': 'Tue, 28 Feb 2023 10:19:00 GMT',
        'Cache-Control': 'must-revalidate',
        'Host': '67.159.88.155:5000',
        'ETag': '10000',
    }
    # test Expires header(expired) w/o cache-control
    # cached, but requires re-validation
    # in cache, but expired at EXPIREDTIME
    # https://edstem.org/us/courses/32653/discussion/2675051
    headers_4 = {
        'Content-Type': 'application/json',
        'Date': 'Tue, 21 Oct 2014 07:28:00 GMT',
        'Expires': 'Wed, 21 Oct 2015 07:28:00 GMT',
        'Host': '67.159.88.155:5000',
        'ETag': '10000',
    }
    # test Expires header(not expired) w/o cache-control
    # cached, expires at EXPIRES
    # if not expired - in cache, valid
    # if expires - in cache, but expired at EXPIREDTIME 
    # https://edstem.org/us/courses/32653/discussion/2675051
    headers_5 = {
        'Content-Type': 'application/json',
        'Date': 'Tue, 21 Oct 2014 07:28:00 GMT',
        'Expires': 'Tue, 28 Feb 2023 04:41:30 GMT',
        'Host': '67.159.88.155:5000',
        'ETag': '10000',
    }
    # test max-age(expired) w/ must-revalidate
    # cached, but requires re-validation
    # in cache, requires validation
    headers_6 = {
        'Content-Type': 'application/json',
        'Date': 'Wed, 21 Oct 2014 07:28:00 GMT',
        'Cache-Control': 'max-age=1, must-revalidate',
        'Host': '67.159.88.155:5000',
        'ETag': '10000',
    }
    # test max-age(expired) w/ proxy-revalidate
    # cached, but requires re-validation
    # in cache, requires validation
    headers_7 = {
        'Content-Type': 'application/json',
        'Date': 'Tue, 28 Feb 2023 10:28:00 GMT',
        'Cache-Control': 'max-age=0, proxy-revalidate',
        'Host': '67.159.88.155:5000',
        'ETag': '10000',
    }
    # test max-age(expired) w/o must/proxy-revalidate
    # cached, but requires re-validation
    # in cache, but expired at EXPIREDTIME 
    headers_8 = {
        'Content-Type': 'application/json',
        'Date': 'Wed, 21 Oct 2014 07:28:00 GMT',
        'Cache-Control': 'max-age=1',
        'Host': '67.159.88.155:5000',
        'ETag': '10000',
    }
    # test max-age(not expired) w/ must-revalidate
    # cached, expires at EXPIRES
    # if not expired - in cache, valid
    # if expired - in cache, requires validation
    headers_9 = {
        'Content-Type': 'application/json',
        'Date': 'Sun, 26 Feb 2023 07:28:00 GMT',
        'Cache-Control': 'max-age=6048000, must-revalidate', # check if expire at MAY 2023
        'Host': '67.159.88.155:5000',
        'ETag': '10000',
    }
    # test max-age(not expired) w/o must-revalidate
    # cached, expires at EXPIRES
    # if not expired - in cache, valid
    # if expired - in cache, but expired at EXPIREDTIME
    headers_10 = {
        'Content-Type': 'application/json',
        'Date': 'Tue, 28 Feb 2023 03:16:00 GMT',
        'Cache-Control': 'max-age=7200',
        'Host': '67.159.88.155:5000',
        'ETag': '10000',
    }
    # test s-maxage(expired) w/ must-revalidate
    # cached, but requires re-validation
    # in cache, requires validation
    headers_11 = {
        'Content-Type': 'application/json',
        'Date': 'Tue, 21 Oct 2014 07:28:00 GMT',
        'Cache-Control': 's-maxage=1, must-revalidate',
        'Host': '67.159.88.155:5000',
        'ETag': '10000',
    }
    # test s-maxage(expired) w/ proxy-revalidate
    # cached, but requires re-validation
    # in cache, requires validation
    headers_12 = {
        'Content-Type': 'application/json',
        'Date': 'Tue, 21 Oct 2014 07:28:00 GMT',
        'Cache-Control': 's-maxage=1, proxy-revalidate',
        'Host': '67.159.88.155:5000',
        'ETag': '10000',
    }
    # test s-maxage(expired) w/o must/proxy-revalidate
    # cached, but requires re-validation
    # in cache, but expired at EXPIREDTIME
    headers_13 = {
        'Content-Type': 'application/json',
        'Date': 'Tue, 21 Oct 2014 07:28:00 GMT',
        'Cache-Control': 's-maxage=1',
        'Host': '67.159.88.155:5000',
        'ETag': '10000',
    }
    # test s-maxage(not expired) w/ must-revalidate
    # cached, expires at EXPIRES
    # if not expired - in cache, valid
    # if expired - in cache, requires validation
    headers_14 = {
        'Content-Type': 'application/json',
        'Date': 'Tue, 28 Feb 2023 05:23:00 GMT',
        'Cache-Control': 's-maxage=17, must-revalidate',
        'Host': '67.159.88.155:5000',
        'ETag': '10000',
    }
    # test s-maxage(not expired) w/o must-revalidate
    # cached, expires at EXPIRES
    # if not expired - in cache, valid
    # if expired - in cache, but expired at EXPIREDTIME
    headers_15 = {
        'Content-Type': 'application/json',
        'Date': 'Tue, 28 Feb 2025 05:25:00 GMT',
        'Cache-Control': 's-maxage=17',
        'Host': '67.159.88.155:5000',
        'ETag': '10000',
    }
    # test cache-control: private
    # not cacheable because REASON (private)
    # not in cache
    headers_16 = {
        'Content-Type': 'application/json',
        'Cache-Control': 'private',
        'Host': '67.159.88.155:5000',
    }
    # test cache-control: no-store
    # not cacheable because REASON (no-store)
    # not in cache
    headers_17 = {
        'Content-Type': 'application/json',
        'Cache-Control': 'no-store',
        'Host': '67.159.88.155:5000',
    }
    # test if Date is not included in the header, cache will add the Date with the received time 
    # cached, expires at EXPIREDTIME
    # in cache, valid
    # after 20s, 
    # in cache, but expired at EXPIREDTIME
    headers_18 = {
        'Content-Type': 'application/json',
        'Cache-Control': 'max-age=20',
        'Host': '67.159.88.155:5000',
    }
     # test if Host is not exist
     # WARNING No host name in the response received from origin server will appear at once
    headers_19 = {
        'Content-Type': 'application/json',
        'Date': 'Tue, 28 Feb 2023 05:35:00 GMT',
        'Cache-Control': 's-maxage=17',
    }

     # test revalidate msg format
    headers_20 = {
        'Content-Type': 'application/json',
        'Date': 'Tue, 28 Feb 2023 05:49:00 GMT',
        'Cache-Control': 's-maxage=18 no-cache',
        'Etag': '"12345"',
        'Host': '67.159.88.155:5000',
        'Last-Modified': 'Tue, 22 Feb 2022 22:00:00 GMT',
    }
    # Finished: malformed-response Date Host ETag/Last-modified Expired/max-age/s-maxage
    # TODO: test in cache, but expired at EXPIREDTIME
    # Finished TODO: test response that no-longer being cached does not cause segfault
    
    headers_test = headers_3      #1,9,15
    response = make_response(jsonify(headers_test), 200) # here to change status code
    response.headers = headers_test # set headers

    return response

if __name__ == '__main__':
    app.run(host='0.0.0.0')