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
    return SOAP_WSDD_ADHOC;
}



void send_hello(struct soap* soap)
{
    struct wsdd_param_t *wsdd_parm = (struct wsdd_param_t *)soap->user;

    if(!wsdd_parm)
        return;

    int res = soap_wsdd_Hello(soap,
                              SOAP_WSDD_ADHOC,             // mode
                              SOAP_WSDD_TS_ADDRESS,        // address of TS
                              soap_wsa_rand_uuid(soap),    // message ID
                              NULL,
                              wsdd_parm->endpoint,
                              wsdd_parm->type,
                              wsdd_parm->scope,
                              NULL,
                              wsdd_parm->xaddr,
                              wsdd_parm->metadata_ver);

    if(res != SOAP_OK)
        soap_print_fault(soap, stderr);
}
