#import <Cocoa/Cocoa.h>
#import <Skype/Skype.h>

@interface SKDelegate : SkypeAPI <SkypeAPIDelegate>
{
	NSString *clientApplicationName;
	NSMutableArray *notificationStrings;
	NSUInteger _capacity;
	bool _isDelegateAttached;
}

- (id)initWithClientApplicationName:(NSString*)name capacity:(NSUInteger)capacity;

- (NSArray *)copyNotificationStrings;

- (void)clearNotificationStrings;

- (bool)isAttached;

@property (nonatomic, retain) NSString *clientApplicationName;

@end
