#import <Foundation/Foundation.h>

#include <string>

namespace ohmydj {
namespace detail {

// macOS HTTP GET via NSURLSession. Uses the system's native TLS, so it works
// inside OBS even though OBS ships no Qt TLS backend. Blocking — call off the
// GUI thread; the semaphore parks this worker thread, not the UI.
std::string HttpGet(const std::string &url)
{
	@autoreleasepool {
		NSString *urlString = [NSString stringWithUTF8String:url.c_str()];
		NSURL *nsurl = urlString ? [NSURL URLWithString:urlString] : nil;
		if (!nsurl)
			return {};

		NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:nsurl];
		[request setValue:@"oh-my-dj" forHTTPHeaderField:@"User-Agent"];
		[request setValue:@"application/vnd.github+json" forHTTPHeaderField:@"Accept"];
		request.timeoutInterval = 8.0;

		__block std::string body;
		dispatch_semaphore_t done = dispatch_semaphore_create(0);
		NSURLSessionDataTask *task = [[NSURLSession sharedSession]
			dataTaskWithRequest:request
			  completionHandler:^(NSData *data, NSURLResponse *response, NSError *error) {
				  NSHTTPURLResponse *http =
					  [response isKindOfClass:[NSHTTPURLResponse class]]
						  ? (NSHTTPURLResponse *)response
						  : nil;
				  if (!error && data && http && http.statusCode == 200)
					  body.assign(static_cast<const char *>(data.bytes), data.length);
				  dispatch_semaphore_signal(done);
			  }];
		[task resume];
		dispatch_semaphore_wait(done, dispatch_time(DISPATCH_TIME_NOW, (int64_t)(10.0 * NSEC_PER_SEC)));
		return body;
	}
}

} // namespace detail
} // namespace ohmydj
