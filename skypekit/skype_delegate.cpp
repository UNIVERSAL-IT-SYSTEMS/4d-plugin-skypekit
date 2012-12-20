#include "skype_delegate.h"
#import "4DPluginAPI.h"

@implementation SKDelegate

@synthesize clientApplicationName;

- (id)initWithClientApplicationName:(NSString*)name capacity:(NSUInteger)capacity
{
	if(!(self = [super init]))return self;
	
	self.clientApplicationName = name;
	
	_capacity = capacity;
	_isDelegateAttached = false;
	
	notificationStrings = [[NSMutableArray alloc]initWithCapacity:capacity];
	
	return self;
}

- (NSArray *)copyNotificationStrings
{
	return (NSArray *)[notificationStrings copy];
}

- (void)clearNotificationStrings
{
	[notificationStrings removeAllObjects];
}

- (void)skypeNotificationReceived:(NSString*)aNotificationString
{
	if(aNotificationString)
	{
		if([notificationStrings count] == _capacity) 
			[notificationStrings removeObjectAtIndex:0];
	
		[notificationStrings addObject:aNotificationString];
	}
}

- (void)skypeAttachResponse:(unsigned)aAttachResponseCode
{
	_isDelegateAttached = aAttachResponseCode;
}

- (bool)isAttached
{
	return _isDelegateAttached;
}

- (void)skypeBecameUnavailable:(NSNotification*)aNotification
{
	_isDelegateAttached = false;
}

- (void)dealloc 
{
	[notificationStrings release];
    [super dealloc];	
}

@end