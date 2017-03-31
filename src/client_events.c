#include <stddef.h>


#include "wsddapi.h"
#include "wsdd_param.h"





void wsdd_event_ProbeMatches(struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, struct wsdd__ProbeMatchesType *matches)
{

}



void wsdd_event_ResolveMatches(struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, struct wsdd__ResolveMatchType *match)
{

}



void wsdd_event_Hello(struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, const char *EndpointReference, const char *Types, const char *Scopes, const char *MatchBy, const char *XAddrs, unsigned int MetadataVersion)
{

}



void wsdd_event_Bye(struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, const char *EndpointReference, const char *Types, const char *Scopes, const char *MatchBy, const char *XAddrs, unsigned int *MetadataVersion)
{

}



soap_wsdd_mode wsdd_event_Resolve(struct soap *soap, const char *MessageID, const char *ReplyTo, const char *EndpointReference, struct wsdd__ResolveMatchType *match)
{
    return SOAP_WSDD_ADHOC;
}



soap_wsdd_mode wsdd_event_Probe(struct soap *soap, const char *MessageID, const char *ReplyTo, const char *Types, const char *Scopes, const char *MatchBy, struct wsdd__ProbeMatchesType *matches)
{
    struct wsdd_param_t *wsdd_param = (struct wsdd_param_t *)soap->user;

    if(!wsdd_param)
        return SOAP_WSDD_ADHOC;


    soap_wsdd_init_ProbeMatches(soap, matches);

    soap_wsdd_add_ProbeMatch(soap,
                             matches,
                             wsdd_param->endpoint,
                             wsdd_param->type,
                             wsdd_param->scope,
                             NULL,
                             get_xaddr(wsdd_param),
                             wsdd_param->metadata_ver);

    soap_wsdd_ProbeMatches(soap,
                           NULL,
                           soap_wsa_rand_uuid(soap) ,
                           MessageID,
                           ReplyTo,
                           matches);


    return SOAP_WSDD_ADHOC;
}
