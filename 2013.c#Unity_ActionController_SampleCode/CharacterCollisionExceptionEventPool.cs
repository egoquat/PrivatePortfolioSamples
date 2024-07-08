#define FIX_KOR_AIRSHARK_IGNORCOLLISION_BUG_20121129

using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;

public class CharacterCollisionExceptionEventPool : MonoBehaviour
{
    protected class CollisionPair
    {
        public CharacterController c1;
        public CharacterController c2;

        public bool Equal(CharacterController c1, CharacterController c2)
        {
            if (this.c1 == c1 && this.c2 == c2 || this.c1 == c2 && this.c2 == c1) return true;
            return false;
        }

        public CollisionPair(CharacterController c1, CharacterController c2)
        {
            this.c1 = c1;
            this.c2 = c2;
        }
    }

    [SerializeField]
    protected float radiusExceptionCollider = 1.0f;
    public float RadiusExceptionCollider
    {
        get { return radiusExceptionCollider; }
    }

    [SerializeField]
    protected string layerNameCollisionException = "Character";
    public string LayerNameCollisionException
    {
        get { return layerNameCollisionException; }
    }

    private float diffCheckDistance = 1.0f;
    CharacterTeamRelationHandler teamColliderRelation = null;

    List<CollisionPair> collisionExceptions = new List<CollisionPair>();

    public float DiffCheckDistance { get { return diffCheckDistance; } }

    //@ Function add to Collision Event List
    public bool NewCollisionException(CharacterController c1, CharacterController c2)
    {
        if (null == c1 || null == c2) { return false; }
        if (collisionExceptions.Exists(p=>p.Equal(c1,c2)))      //@Already exists on collision exception list
        {
            return false;
        }

        CustomCharacterMotor c1Motor = c1.GetComponent<CustomCharacterMotor>();
        if (null == c1Motor) { return false; }
        CustomCharacterMotor c2Motor = c2.GetComponent<CustomCharacterMotor>();
        if (null == c2Motor) { return false; }

        if (true == c1Motor.IsGrounded() && c1.isGrounded)
        {
            return false;
        }

        //@이미 팀 상호간 충돌 예외 리스트업 되어 있는가 여부.
        CharacterActionBrain a1 = c1.GetComponent<CharacterActionBrain>();
        CharacterActionBrain a2 = c2.GetComponent<CharacterActionBrain>();
        if (true == teamColliderRelation.isExistExceptionRelation(a1, a2)) return false;
        //Identifier i1 = c1.GetComponent<Identifier>();
        //Identifier i2 = c2.GetComponent<Identifier>();
        //if (null != i1 && null != i2)
        //{
        //    if (true == i1.isAllyTeamWith(i2)) { return false; }
        //}

        //Debug.Log("==========> Physics.IgnoreCollision c2(" + (c2)
        //    + ")/c1.isGrounded(" + (c1.isGrounded) + ")/c1Motor.IsGrounded(" + (c1Motor.IsGrounded()) + ")/" + Time.time);

        if (false == c1.collider.enabled || false == c2.collider.enabled) { return false; }

#if FIX_KOR_AIRSHARK_IGNORCOLLISION_BUG_20121129

        if (c1.collider.enabled == false || c2.collider.enabled == false )
        {
            return false;
        }

        Physics.IgnoreCollision(c1.collider, c2.collider);
#else
        Physics.IgnoreCollision(c1.collider, c2.collider);
#endif

        collisionExceptions.Add(new CollisionPair(c1, c2));
        return true;
    }

    public void RecoverCollisionException(CharacterController c1, CharacterController c2)
    {
        if (null == c1 || null == c2) { return; }
        if (!collisionExceptions.Exists(p => p.Equal(c1, c2)))
        {
            return;
        }

        List<CollisionPair> removelist = new List<CollisionPair>();
        collisionExceptions.Each(k=>
        {
            if (k.Equal(c1, c2))
            {
                removelist.Add(k);
            }
        });       

        if (removelist.Count < 1) return;
        bool removedLeastOneMore = false;
        foreach (CollisionPair c in removelist) 
        {
            if (true == collisionExceptions.Remove(c)) { removedLeastOneMore = true; }
        }

        if (true == removedLeastOneMore)
        {
#if FIX_KOR_AIRSHARK_IGNORCOLLISION_BUG_20121129
            
            if (c1.collider.enabled == false || c2.collider.enabled == false)
            {
                return;
            }

            Physics.IgnoreCollision(c1.collider, c2.collider, false);
#else
            Physics.IgnoreCollision(c1.collider, c2.collider, false);
#endif
        }
    }

    public void RecoverAllCollisionException()
    {
        foreach (CollisionPair p in collisionExceptions)
        {
#if FIX_KOR_AIRSHARK_IGNORCOLLISION_BUG_20121129
            if (p.c1.collider.enabled == false || p.c2.collider.enabled == false )
            {
                continue;
            }

            Physics.IgnoreCollision(p.c1.collider, p.c2.collider, false);
#else
            Physics.IgnoreCollision(p.c1.collider, p.c2.collider, false);
#endif
        }

        collisionExceptions.Clear();
    } 

    // Use this for initialization
    void Awake()
    {
        teamColliderRelation = GetComponent<CharacterTeamRelationHandler>();
        if (null == teamColliderRelation) { Debug.LogError("(null == teamColliderRelation)"); }
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}

